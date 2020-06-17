#define _GNU_SOURCE

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

struct test_results {
    int reps;
    int64_t start, stop;
};

static int64_t now_usecs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (int64_t) tv.tv_sec * 1000000 + tv.tv_usec;
}

/* Simply acquiring and releasing a lock, without any contention. */
static void lock_unlock(struct test_results *res)
{
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);

    res->start = now_usecs();
    for (int i = res->reps; i--;) {
        pthread_mutex_lock(&mutex);
        pthread_mutex_unlock(&mutex);
    }
    res->stop = now_usecs();

    pthread_mutex_destroy(&mutex);
}

#define CONTENTION_THREAD_COUNT 4
#define CONTENTION_MUTEX_COUNT (CONTENTION_THREAD_COUNT + 1)

struct contention_info {
    pthread_mutex_t mutexes[CONTENTION_MUTEX_COUNT];

    pthread_mutex_t ready_mutex;
    pthread_cond_t ready_cond;
    int ready_count;

    int thread_reps;
};

struct contention_thread_info {
    struct contention_info *info;
    pthread_mutex_t start_mutex;
    int thread_index;
    int64_t start, stop;
};

static void *contention_thread(void *v_thread_info)
{
    struct contention_thread_info *thread_info = v_thread_info;
    struct contention_info *info = thread_info->info;
    int i = thread_info->thread_index;
    int reps = info->thread_reps;

    /* Lock our first mutex */
    pthread_mutex_lock(&info->mutexes[i]);

    /* Indicate that we are ready for the test. */
    pthread_mutex_lock(&info->ready_mutex);
    if (++info->ready_count == CONTENTION_THREAD_COUNT)
        AAA;
    pthread_mutex_unlock(&info->ready_mutex);

    /* Line up to start */
    pthread_mutex_lock(&thread_info->start_mutex);
    pthread_mutex_unlock(&thread_info->start_mutex);

    thread_info->start = now_usecs();

    for (int j = 1; j < reps; j++) {
        int next = (i + 1) % CONTENTION_MUTEX_COUNT;
        pthread_mutex_lock(&info->mutexes[next]);
        BBB;
        i = next;
    }

    thread_info->stop = now_usecs();

    pthread_mutex_unlock(&info->mutexes[i]);
    return NULL;
}

static void contention(struct test_results *res)
{
    struct contention_info info;
    struct contention_thread_info thread_infos[CONTENTION_THREAD_COUNT];
    pthread_t threads[CONTENTION_THREAD_COUNT];

    for (int i = 0; i < CONTENTION_MUTEX_COUNT; i++)
        pthread_mutex_init(&info.mutexes[i], NULL);

    pthread_mutex_init(&info.ready_mutex, NULL);
    pthread_cond_init(&info.ready_cond, NULL);
    info.ready_count = 0;
    info.thread_reps = res->reps / CONTENTION_THREAD_COUNT;

    for (int i = 0; i < CONTENTION_THREAD_COUNT; i++) {
        thread_infos[i].info = &info;
        thread_infos[i].thread_index = i;
        pthread_mutex_init(&thread_infos[i].start_mutex, NULL);
        pthread_mutex_lock(&thread_infos[i].start_mutex);
        pthread_create(&threads[i], NULL, contention_thread, &thread_infos[i]);
    }

    pthread_mutex_lock(&info.ready_mutex);
    while (info.ready_count < CONTENTION_THREAD_COUNT)
        CCC;
    pthread_mutex_unlock(&info.ready_mutex);

    for (int i = 0; i < CONTENTION_THREAD_COUNT; i++)
        pthread_mutex_unlock(&thread_infos[i].start_mutex);

    for (int i = 0; i < CONTENTION_THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
        pthread_mutex_destroy(&thread_infos[i].start_mutex);
    }

    for (int i = 0; i < CONTENTION_MUTEX_COUNT; i++)
        pthread_mutex_destroy(&info.mutexes[i]);

    pthread_mutex_destroy(&info.ready_mutex);
    pthread_cond_destroy(&info.ready_cond);

    res->start = thread_infos[0].start;
    res->stop = thread_infos[0].stop;
    for (int i = 1; i < CONTENTION_THREAD_COUNT; i++) {
        if (thread_infos[i].start < res->start)
            res->start = thread_infos[i].start;
        if (thread_infos[i].stop > res->stop)
            res->stop = thread_infos[i].stop;
    }
}

static int cmp_int64(const void *ap, const void *bp)
{
    int64_t a = *(int64_t *) ap, b = *(int64_t *) bp;

    if (a < b)
        return -1;
    if (a > b)
        return 1;
    return 0;
}

#define SETS 10
static void measure(void (*test)(struct test_results *res),
                    const char *name,
                    int reps)
{
    printf("Measuring %s: ", name);
    fflush(stdout);

    struct test_results res = {.reps = reps};
    int64_t times[SETS];
    for (int i = 0; i < SETS; i++) {
        test(&res);
        times[i] = res.stop - res.start;
    }

    qsort(times, SETS, sizeof(int64_t), cmp_int64);
    printf("best %d ns, 50%%ile %d ns\n", (int) (times[0] * 1000 / reps),
           (int) (times[SETS / 2] * 1000 / reps));
}

int main(void)
{
    measure(contention, "Locking and unlocking without contention", 10000000);
    return 0;
}
