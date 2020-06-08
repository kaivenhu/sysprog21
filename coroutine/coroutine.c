/*
 * Stackless coroutines.
 * This is an async/await implementation for C based on Duff's device.
 *
 * Features:
 * 1. the async state is caller-saved rather than callee-saved.
 * 2. Subroutines can have persistent state that is not just static state,
 *    because each async subroutine accepts its own struct it uses as a
 *    parameter, and the async state is stored there.
 * 3. Because of the more flexible state, async subroutines can be nested
 *    in tree-like fashion which permits fork/join concurrency patterns.
 */

/* event status */
typedef enum ASYNC_EVT { ASYNC_INIT = 0, ASYNC_DONE = -1 } async;

/* Declare the async state */
#define async_state unsigned int _async_kcont
struct async {
    async_state;
};

/* Mark the start of an async subroutine */
#define async_begin(k) XXX1 XXX2

/* Mark the end of a generator thread */
#define async_end          \
    default:               \
        return ASYNC_DONE; \
        }

/* Wait until the condition succeeds */
#define await(cond) await_while(!(cond))

/* Wait while the condition succeeds */
#define await_while(cond) \
    case __LINE__:        \
        if (cond)         \
        return __LINE__

/* Initialize a new async computation */
#define async_init(state) (state)->_async_kcont = ASYNC_INIT

/* Check if async subroutine is done */
#define async_done(state) (state)->_async_kcont == ASYNC_DONE

/* Resume a running async computation and check for completion */
#define async_call(f, state) \
    (async_done(state) || ASYNC_DONE == ((state)->_async_kcont = (f)(state)))

struct async_sem {
    unsigned int count;
};

/**
 * Initialize a semaphore
 *
 * This macro initializes a semaphore with a value for the
 * counter. Internally, the semaphores use an "unsigned int" to
 * represent the counter, and therefore the "count" argument should be
 * within range of an unsigned int.
 *
 * \param s A pointer to the object representing the semaphore.
 * \param c (unsigned int) The initial count of the semaphore.
 */
#define init_sem(s, c) (s)->count = c

/**
 * Wait for a semaphore
 *
 * This macro carries out the "wait" operation on the semaphore. The
 * wait operation causes the protothread to block while the counter is
 * zero. When the counter reaches a value larger than zero, the
 * protothread will continue.
 *
 * \param s A pointer to the object representing the semaphore.
 */
#define await_sem(s)           \
    do {                       \
        await((s)->count > 0); \
        XXX3;                  \
    } while (0)

/**
 * Signal a semaphore
 *
 * This macro carries out the "signal" operation on the semaphore. The
 * signal operation increments the counter inside the semaphore, which
 * eventually will cause waiting protothreads to continue executing.
 *
 * \param s A pointer to the object representing the semaphore.
 */
#define signal_sem(s) XXX4

/* Use case */

#include <stdio.h>
#include <unistd.h>

#define NUM_ITEMS 32
#define BUFSIZE 8

static int buffer[BUFSIZE];
static int bufptr;

static void add_to_buffer(int item)
{
    printf("Item %d added to buffer at place %d\n", item, bufptr);
    buffer[bufptr] = item;
    bufptr = (bufptr + 1) % BUFSIZE;
}
static int get_from_buffer(void)
{
    int item = buffer[bufptr];
    printf("Item %d retrieved from buffer at place %d\n", item, bufptr);
    bufptr = (bufptr + 1) % BUFSIZE;
    return item;
}

static int produce_item(void)
{
    static int item = 0;
    printf("Item %d produced\n", item);
    return item++;
}

static void consume_item(int item)
{
    printf("Item %d consumed\n", item);
}

static struct async_sem full, empty;

static async producer(struct async *pt)
{
    static int produced;

    async_begin(pt);
    for (produced = 0; produced < NUM_ITEMS; ++produced) {
        await_sem(&full);

        add_to_buffer(produce_item());

        signal_sem(&empty);
    }
    async_end;
}

static async consumer(struct async *pt)
{
    static int consumed;

    async_begin(pt);
    for (consumed = 0; consumed < NUM_ITEMS; ++consumed) {
        await_sem(&empty);
        consume_item(get_from_buffer());
        signal_sem(&full);
    }
    async_end;
}

static async driver_thread(struct async *pt)
{
    static struct async cr_producer, cr_consumer;

    async_begin(pt);

    init_sem(&empty, 0);
    init_sem(&full, BUFSIZE);

    async_init(&cr_producer);
    async_init(&cr_consumer);

    await(producer(&cr_producer) & consumer(&cr_consumer));

    async_end;
}

int main(void)
{
    struct async driver_pt;
    async_init(&driver_pt);

    while (!async_call(driver_thread, &driver_pt)) {
        usleep(10); /* why? */
    }
    return 0;
}
