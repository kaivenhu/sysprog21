#include <inttypes.h>
#include <sys/time.h>

enum {
    EV_READ = (1 << 0),
    EV_WRITE = (1 << 1),
    EV_TIMEOUT_ONESHOT = (1 << 2),
    EV_TIMEOUT_PERIODIC = (1 << 3),
    EV_SIGNAL = (1 << 4),
    EV_CLOEXEC = (1 << 0),
};

/*
 * Forward declaration - ev objects are fully opaque to callers. Access is
 * always done via access functions.  Just keep in mind: ev is the object you
 * usualy needs one, the main object. For each registered event like timer or
 * file descriptor you corresponding ev_entry object is required.
 */
struct ev;
struct ev_entry;

/**
 * ev_new - initialize a new event object, eve main data structure
 *
 * It return the new ev object or NULL in the case of an error.
 */
struct ev *ev_new(int flags);

/**
 * Add ev_event to the main ev event structure
 *
 * This registers all events (fd, signals, timer) at the main structure and
 * must be called before ev_loop.
 *
 * In the case of an error an negative errno value is returned.  It is up to
 * the caller to free ev_entry data structure.
 */
int ev_add(struct ev *, struct ev_entry *);

/**
 * Main event loop start function
 *
 * This function will call epoll_wait and will block until event is triggered.
 * Please call this at the end after every ev_event's are registered.
 */
int ev_loop(struct ev *, int);

/* To end the processing loop
 *
 * Keep in mind: this will not free any memory, nor does this function call
 * ev_del to deregister.  It just break out after an event it triggered.
 *
 * This function is probably not what you want to use
 */
int ev_run_out(struct ev *);

/**
 * ev_destroy - deallocate ev structure
 * @ev: pointer instance of ev object
 *
 * Shutdown, close and free all associated resourches of ev. This function
 * it the counterpart to ev_new() and should at least be called at
 * program shutdown or restart.
 *
 * Keep in mind that the caller is responsible to deallocate all registered
 * ev_event data structures, close file descriptors, etc. This cannot be done
 * by ev_destroy().
 *
 * This function cannot fail and thus return no return status.
 */
void ev_destroy(struct ev *);

/**
 * ev_entry new provides api to register a raw filedescriptor (e.g. socket)
 * for later use in epoll set. The arguments:
 *
 * 1) the filedescriptor
 * 2) EV_READ or EV_WRITE
 * 3) a callback, called if fd is ready for read or write
 * 4) a private data hand over to the caller within the callback
 *
 * The callback protoype is similar:
 *
 * 1) the filedescriptor
 * 2) EV_READ or EV_WRITE
 * 3) the private data pointer, registered at ev_entry_new time
 *
 * Warning: do not throw exceptions or call longjmp from a callback.
 *
 * The next steps is to register this entry at the main loop wia
 * ev_add()
 *
 * This function return NULL in the case of an error or a pointer
 * to a newly allocated struct.
 */
struct ev_entry *ev_entry_new_raw(int,
                                  uint32_t,
                                  void (*cb)(int, uint32_t, void *),
                                  void *);

/**
 * Deregister event from main event loop
 *
 * Please make sure to call ev_entry_free() to remove all allocated
 * resourches to free memory.
 *
 * Return 0 in the case of sucess, otherwise a negative error code.
 */
int ev_del(struct ev *, struct ev_entry *);

/**
 * Deallocate resourcheso of ev_eventy
 *
 * This is the counterpart of ev_entry_new(), ev_timer_oneshot_new(),
 * and ev_timer_periodic_new() must be called to free associated memory.
 */
void ev_entry_free(struct ev_entry *);

/**
 * Create new oneshot timer
 *
 * Just arm a timer for one shot, after the callback the timer is not re-added
 * to the main loop automatically.  The caller is responsible to free
 * resourches afterwards with ev_entry_free()
 *
 * Warning: do not throw exceptions or call longjmp from a callback.
 */
struct ev_entry *ev_timer_oneshot_new(struct timespec *,
                                      void (*cb)(void *),
                                      void *);

/**
 * Start periodic timer
 *
 * Ater timespec time the user provided callack cb is called. To end the timer
 * ev_timer_cancel() must be called. Normally followed by ev_entry_free()
 *
 * NOTE: the first callback argument tell the number of missed events. This
 * can happen if too much work is scheduled and the even machinery cannot
 * execute fast enough. Normally you should only see 1.
 *
 * Warning: do not throw exceptions or call longjmp from a callback.
 *
 * Returns NULL in case the case of an error
 */
struct ev_entry *ev_timer_periodic_new(struct timespec *,
                                       void (*cb)(unsigned long long missed,
                                                  void *),
                                       void *);

/*
 * struct ev_event * is freed by ev_timer_cancel - user provided callbacks
 * and data not - sure. So do not dereference ev_entry afterwards
 *
 * Make sure that ever you cancel the timer you call ev_entry_free()
 *
 */
int ev_timer_cancel(struct ev *, struct ev_entry *);

/**
 * set filedescriptor in non-blocking mode
 *
 * All descriptors registered at epoll must be operate in non-blocking
 * way. Often you can get a non-blocking descriptor with the right options
 * for some syscalls. If not this function can be used.
 *
 * This function return the 0 in the case of success or negative value
 * if something went wront.
 */
int ev_set_non_blocking(int fd);

/* Implementation starts here */

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/timerfd.h>

#define EVE_EPOLL_ARRAY_SIZE 64

struct ev {
    int fd;
    int break_loop;
    unsigned long long entries;

    /* implementation specific data, e.g. select timer handling
     * will use this to store the rbtree */
    void *priv_data;
};

struct ev_entry {
    /* monitored FD if type is EV_READ or EV_WRITE */
    int fd;

    /* EV_* if raw is 0 -> type is used. E.g for
     * EV_READ, EV_WRITE or EV_TIMEOUT_ONESHOT.\
     * EV_RAW_* if raw is 1 -> type_raw is used then
     */
    union {
        int type;
        uint32_t type_raw;
    };

    /* 0 for "old" mode, if 1 type is interpreted identical
     * as epoll_ctl flags */
    int raw;

    /* timeout val if type is EV_TIMEOUT_ONESHOT */
    struct timespec timespec;

    union {
        void (*fd_cb)(int, int, void *);
        void (*fd_cb_raw)(int, uint32_t, void *);
        void (*timer_cb_oneshot)(void *);
        void (*timer_cb_periodic)(unsigned long long, void *);
        void (*signal_cb)(uint32_t, uint32_t, void *);
    };

    /* user provided pointer to data */
    void *data;

    /* implementation specific data (e.g. for epoll, select) */
    void *priv_data;
};

static struct ev *struct_ev_new_internal(void)
{
    struct ev *ev = malloc(sizeof(*ev));
    if (!ev)
        return NULL;

    memset(ev, 0, sizeof(*ev));
    return ev;
}

int ev_run_out(struct ev *ev)
{
    ev->break_loop = 1;
    return 0;
}

/* similar for all implementations, at least
 * under Linux. Solaris, AIX, etc. differs and need
 * a separate implementation */
int ev_set_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return -EINVAL;

    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    if (flags < 0)
        return -EINVAL;

    return 0;
}

struct ev_entry_data_epoll {
    /* std fd handling data */
    uint32_t flags;
    union {
        sigset_t signal_mask;
    };
};

void ev_destroy(struct ev *ev)
{
    /* close epoll descriptor */
    close(ev->fd);

    /* clear potential secure data */
    memset(ev, 0, sizeof(struct ev));
    free(ev);
}

static inline int ev_new_flags_convert(int flags)
{
    if (flags == 0)
        return 0;
    if (flags == EV_CLOEXEC)
        return EPOLL_CLOEXEC;
    return -EINVAL;
}

struct ev *ev_new(int flags)
{
    int flags_epoll = ev_new_flags_convert(flags);
    if (flags_epoll < 0)
        return NULL;

    struct ev *ev = struct_ev_new_internal();
    if (!ev)
        return NULL;

    ev->fd = epoll_create1(flags_epoll);
    if (ev->fd < 0) {
        free(ev);
        return NULL;
    }

    ev->entries = 0;
    ev->break_loop = 0;
    return ev;
}

struct ev_entry *ev_entry_new_epoll_internal(void)
{
    struct ev_entry *ev_entry = malloc(sizeof(struct ev_entry));
    if (!ev_entry)
        return NULL;

    memset(ev_entry, 0, sizeof(struct ev_entry));

    ev_entry->priv_data = malloc(sizeof(struct ev_entry_data_epoll));
    if (!ev_entry->priv_data) {
        free(ev_entry);
        return NULL;
    }

    memset(ev_entry->priv_data, 0, sizeof(struct ev_entry_data_epoll));
    return ev_entry;
}

struct ev_entry *ev_entry_new_raw(int fd,
                                  uint32_t events,
                                  void (*cb)(int, uint32_t, void *),
                                  void *data)
{
    struct ev_entry *ev_entry = ev_entry_new_epoll_internal();
    if (!ev_entry)
        return NULL;

    ev_entry->fd = fd;
    ev_entry->type_raw = events;
    ev_entry->fd_cb_raw = cb;
    ev_entry->raw = 1;
    ev_entry->data = data;

    struct ev_entry_data_epoll *ev_entry_data_epoll = ev_entry->priv_data;
    ev_entry_data_epoll->flags = events;

    return ev_entry;
}

struct ev_entry *ev_timer_oneshot_new(struct timespec *timespec,
                                      void (*cb)(void *),
                                      void *data)
{
    struct ev_entry *ev_entry = ev_entry_new_epoll_internal();
    if (!ev_entry)
        return NULL;

    ev_entry->type = EV_TIMEOUT_ONESHOT;
    ev_entry->data = data;
    ev_entry->timer_cb_oneshot = cb;
    ev_entry->raw = 0;

    memcpy(&ev_entry->timespec, timespec, sizeof(struct timespec));
    return ev_entry;
}

struct ev_entry *ev_timer_periodic_new(struct timespec *timespec,
                                       void (*cb)(unsigned long long, void *),
                                       void *data)
{
    struct ev_entry *ev_entry = ev_entry_new_epoll_internal();
    if (!ev_entry)
        return NULL;

    ev_entry->type = EV_TIMEOUT_PERIODIC;
    ev_entry->data = data;
    ev_entry->timer_cb_periodic = cb;
    ev_entry->raw = 0;

    memcpy(&ev_entry->timespec, timespec, sizeof(struct timespec));
    return ev_entry;
}

static void ev_entry_timer_free(struct ev_entry *ev_entry)
{
    close(ev_entry->fd);
}

static void ev_entry_signal_free(struct ev_entry *ev_entry)
{
    close(ev_entry->fd);
}

void ev_entry_free(struct ev_entry *ev_entry)
{
    if (ev_entry->raw)
        goto out;

    switch (ev_entry->type) {
    case EV_TIMEOUT_ONESHOT:
    case EV_TIMEOUT_PERIODIC:
        ev_entry_timer_free(ev_entry);
        break;
    case EV_SIGNAL:
        ev_entry_signal_free(ev_entry);
        break;
    default:
        // other events have no special cleaning
        // functions. do nothing
        break;
    }

out:
    free(ev_entry->priv_data);
    memset(ev_entry, 0, sizeof(struct ev_entry));
    free(ev_entry);
}

static int ev_arm_timerfd_oneshot(struct ev_entry *ev_entry)
{
    struct timespec now;
    struct itimerspec new_value;
    struct ev_entry_data_epoll *ev_entry_data_epoll = ev_entry->priv_data;

    memset(&new_value, 0, sizeof(struct itimerspec));

    int ret = clock_gettime(CLOCK_MONOTONIC, &now);
    if (ret < 0)
        return -EINVAL;

    new_value.it_value.tv_sec = now.tv_sec + ev_entry->timespec.tv_sec;
    new_value.it_value.tv_nsec = now.tv_nsec + ev_entry->timespec.tv_nsec;

    /* timerfd_settime() cannot handle larger nsecs - catch overflow */
    if (new_value.it_value.tv_nsec >= 1000000000) {
        new_value.it_value.tv_sec++;
        new_value.it_value.tv_nsec -= 1000000000;
    }

    new_value.it_interval.tv_sec = 0;
    new_value.it_interval.tv_nsec = 0;

    int fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd < 0)
        return -EINVAL;

    ret = timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL);
    if (ret < 0) {
        close(fd);
        return -EINVAL;
    }

    ret = ev_set_non_blocking(fd);
    if (ret < 0) {
        close(fd);
        return -EINVAL;
    }

    ev_entry_data_epoll->flags = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
    ev_entry->fd = fd;
    return 0;
}

static int ev_arm_timerfd_periodic(struct ev_entry *ev_entry)
{
    struct ev_entry_data_epoll *ev_entry_data_epoll = ev_entry->priv_data;
    struct itimerspec new_value = {
        .it_value.tv_sec = ev_entry->timespec.tv_sec,
        .it_value.tv_nsec = ev_entry->timespec.tv_nsec,
        .it_interval.tv_sec = ev_entry->timespec.tv_sec,
        .it_interval.tv_nsec = ev_entry->timespec.tv_nsec,
    };

    int fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd < 0)
        return -EINVAL;

    int ret = timerfd_settime(fd, 0, &new_value, NULL);
    if (ret < 0) {
        close(fd);
        return -EINVAL;
    }

    ret = ev_set_non_blocking(fd);
    if (ret < 0) {
        close(fd);
        return -EINVAL;
    }

    ev_entry_data_epoll->flags = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
    ev_entry->fd = fd;
    return 0;
}

static int ev_arm_signal(struct ev_entry *ev_entry)
{
    struct ev_entry_data_epoll *ev_entry_data_epoll = ev_entry->priv_data;

    int ret = sigprocmask(SIG_BLOCK, &ev_entry_data_epoll->signal_mask, NULL);
    if (ret < 0)
        return -EINVAL;

    int fd = signalfd(-1, &ev_entry_data_epoll->signal_mask, SFD_CLOEXEC);
    if (fd < 0)
        return -EINVAL;

    ret = ev_set_non_blocking(fd);
    if (ret < 0) {
        close(fd);
        return -EINVAL;
    }

    ev_entry_data_epoll->flags = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;
    ev_entry->fd = fd;
    return 0;
}

int ev_add(struct ev *ev, struct ev_entry *ev_entry)
{
    int ret;
    struct epoll_event epoll_ev;
    struct ev_entry_data_epoll *ev_entry_data_epoll = ev_entry->priv_data;
    memset(&epoll_ev, 0, sizeof(struct epoll_event));

    if (ev_entry->raw) {
        /* type is interpreted as raw epoll_ctl event, not special
         * internal event, no special treatment required */
        goto out;
    }

    switch (ev_entry->type) {
    case EV_TIMEOUT_ONESHOT:
        ret = ev_arm_timerfd_oneshot(ev_entry);
        if (ret != 0)
            return -EINVAL;
        break;
    case EV_TIMEOUT_PERIODIC:
        ret = ev_arm_timerfd_periodic(ev_entry);
        if (ret != 0)
            return -EINVAL;
        break;
    case EV_SIGNAL:
        ret = ev_arm_signal(ev_entry);
        if (ret != 0)
            return -EINVAL;
        break;
    default:
        // no special treatment of other entries
        break;
    }

out:
    /* FIXME: the mapping must be a one to one mapping */
    epoll_ev.events = ev_entry_data_epoll->flags;
    epoll_ev.data.ptr = ev_entry;

    ret = epoll_ctl(ev->fd, EPOLL_CTL_ADD, ev_entry->fd, &epoll_ev);
    if (ret < 0)
        return -EINVAL;

    ev->entries++;
    return 0;
}

int ev_del(struct ev *ev, struct ev_entry *ev_entry)
{
    struct epoll_event epoll_ev;
    memset(&epoll_ev, 0, sizeof(struct epoll_event));

    int ret = epoll_ctl(ev->fd, EPOLL_CTL_DEL, ev_entry->fd, &epoll_ev);
    if (ret < 0)
        return -EINVAL;

    ev->entries--;
    return 0;
}

int ev_timer_cancel(struct ev *ev, struct ev_entry *ev_entry)
{
    int ret = ev_del(ev, ev_entry);
    if (ret != 0)
        return -EINVAL;
    return 0;
}

static inline void ev_process_timer_oneshot(struct ev *ev,
                                            struct ev_entry *ev_entry)
{
    unsigned long long missed;

    /* and now: cleanup timer specific data and
     * finally all event specific data */
    ssize_t ret = read(ev_entry->fd, &missed, sizeof(missed));
    if (ret < 0)
        assert(0);

    ev_del(ev, ev_entry);

    /* first of all - call user callback */
    ev_entry->timer_cb_oneshot(ev_entry->data);
}

static inline void ev_process_timer_periodic(struct ev_entry *ev_entry)
{
    unsigned long long missed;

    /* and now: cleanup timer specific data and
     * finally all event specific data */
    ssize_t ret = read(ev_entry->fd, &missed, sizeof(missed));
    if (ret < 0)
        assert(0);

    /* first of all - call user callback */
    ev_entry->timer_cb_periodic(missed, ev_entry->data);
}

static inline void ev_process_signal(struct ev_entry *ev_entry)
{
    struct signalfd_siginfo sigsiginfo;

    /* and now: cleanup timer specific data and
     * finally all event specific data */
    ssize_t ret = read(ev_entry->fd, &sigsiginfo, sizeof(sigsiginfo));
    if (ret < 0) {
        assert(0);
        return;
    }
    if (ret != sizeof(sigsiginfo))
        return;
    ev_entry->signal_cb(sigsiginfo.ssi_signo, sigsiginfo.ssi_pid,
                        ev_entry->data);
}

static inline void ev_process_call_internal(struct ev *ev,
                                            struct ev_entry *ev_entry)
{
    (void) ev;

    if (ev_entry->raw) {
        ev_entry->fd_cb_raw(ev_entry->fd, ev_entry->type_raw, ev_entry->data);
        return;
    }

    switch (ev_entry->type) {
    case EV_READ:
    case EV_WRITE:
        ev_entry->fd_cb(ev_entry->fd, ev_entry->type, ev_entry->data);
        return;
        break;
    case EV_TIMEOUT_ONESHOT:
        ev_process_timer_oneshot(ev, ev_entry);
        break;
    case EV_TIMEOUT_PERIODIC:
        ev_process_timer_periodic(ev_entry);
        break;
    case EV_SIGNAL:
        ev_process_signal(ev_entry);
        break;
    default:
        return;
        break;
    }
    return;
}

int ev_loop(struct ev *ev, int flags)
{
    (void) flags;
    struct epoll_event events[EVE_EPOLL_ARRAY_SIZE];

    while (ev->entries > 0) {
        int nfds = epoll_wait(ev->fd, events, EVE_EPOLL_ARRAY_SIZE, -1);
        if (nfds < 0)
            return -EINVAL;

        /* multiplex and call the registerd callback handler */
        for (int i = 0; i < nfds; i++) {
            struct ev_entry *ev_entry = events[i].data.ptr;
            ev_process_call_internal(ev, ev_entry);
        }

        if (ev->break_loop)
            break;
    }
    return 0;
}

/* Unit test starts here */

#define SLEEP_SECONDS 1
#define ITERATIO_MAX 2

int i = 0;

void timer_cd(void *data)
{
    struct ev *ev = data;
    struct ev_entry *ev_e;
    struct timespec timespec = {SLEEP_SECONDS, 0};

    fprintf(stderr, "timer_cd() called %d time\n", i);
    if (i++ >= ITERATIO_MAX)
        return;

    ev_e = ev_timer_oneshot_new(&timespec, timer_cd, ev);
    if (!ev_e) {
        fprintf(stderr, "failed to create a ev_entry object\n");
        exit(666);
    }

    int ret = ev_add(ev, ev_e);
    if (ret != 0)
        fprintf(stderr, "Cannot add entry to event handler (%d)\n", i);
    return;
}

struct ev_wrapper {
    struct ev *ev;
    struct ev_entry *ev_entry;
};

static void cancel_timer_cb(void *data)
{
    struct ev_wrapper *ev_wrapper = data;

    int ret = ev_timer_cancel(ev_wrapper->ev, ev_wrapper->ev_entry);
    if (ret != 0) {
        fprintf(stderr, "failed to cancel timer\n");
        exit(EXIT_FAILURE);
    }

    ev_entry_free(ev_wrapper->ev_entry);
    return;
}

/* idea, test that timers are called in strict order and that the
 * first timer (fired after 1 seconds) cancel the 5 second timout timer */
static int do_cancel_test(struct ev *ev)
{
    int flags = 0;
    struct ev_entry *eve1, *eve2;
    struct timespec timespec1 = {.tv_sec = 5, .tv_nsec = 0};
    struct timespec timespec2 = {.tv_sec = 1, .tv_nsec = 0};
    struct ev_wrapper *ev_wrapper;

    fprintf(stderr, "run timer cancel test ...");

    eve1 = ev_timer_oneshot_new((void *) &timespec1, (void *) timer_cd,
                                (void *) ev);
    if (!eve1) {
        fprintf(stderr, "Failed to create a ev_entry object\n");
        exit(EXIT_FAILURE);
    }

    int ret = ev_add(ev, eve1);
    if (ret != 0) {
        fprintf(stderr, "Cannot add entry to event handler\n");
        exit(EXIT_FAILURE);
    }

    ev_wrapper = malloc(sizeof(struct ev_wrapper));
    if (!ev_wrapper) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    ev_wrapper->ev = ev;
    ev_wrapper->ev_entry = eve1;

    eve2 = ev_timer_oneshot_new((void *) &timespec2, (void *) cancel_timer_cb,
                                (void *) ev_wrapper);
    if (!eve2) {
        fprintf(stderr, "Failed to create a ev_entry object\n");
        exit(EXIT_FAILURE);
    }

    ret = ev_add(ev, eve2);
    if (ret != 0) {
        fprintf(stderr, "Cannot add entry to event handler\n");
        exit(EXIT_FAILURE);
    }

    ev_loop(ev, flags);

    free(ev_wrapper);

    fprintf(stderr, " passed\n");
    return 1;
}

static void test_timer(void)
{
    struct ev *ev = ev_new(0);
    if (!ev) {
        fprintf(stderr, "Cannot create event handler\n");
        return;
    }

    do_cancel_test(ev);
    ev_destroy(ev);
}

struct ctx_timer {
    struct ev_entry *eve;
    struct ev *ev;
    unsigned periodic_runs;
};

struct ctx_timer *ctx_timer_new(void)
{
    struct ctx_timer *ctxo = malloc(sizeof(*ctxo));
    if (!ctxo)
        abort();
    memset(ctxo, 0, sizeof(*ctxo));
    return ctxo;
}

void callback_oneshot(void *data)
{
    struct ctx_timer *ctxo = data;

    ev_entry_free(ctxo->eve);
    free(ctxo);

    fprintf(stderr, "callback called\n");
}

static void test_timer_oneshot(void)
{
    int flags = 0, ret;
    struct ev_entry *eve;
    struct ctx_timer *ctxo;
    struct timespec ts = {.tv_sec = 1, .tv_nsec = 0};

    fprintf(stderr, "Test: oneshot timer\n");

    struct ev *ev = ev_new(0);
    if (!ev) {
        fprintf(stderr, "Cannot create event handler\n");
        return;
    }

    ctxo = ctx_timer_new();
    ctxo->ev = ev;

    eve = ev_timer_oneshot_new(&ts, callback_oneshot, ctxo);
    if (!eve) {
        fprintf(stderr, "Failed to create a ev_entry object\n");
        exit(EXIT_FAILURE);
    }

    ctxo->eve = eve;

    ret = ev_add(ev, eve);
    if (ret != 0) {
        fprintf(stderr, "Cannot add entry to event handler\n");
        exit(EXIT_FAILURE);
    }

    // ev_loop will run until the timeout is fired. Which
    // in turn is the last event, which will end the ev loop
    ev_loop(ev, flags);

    ev_destroy(ev);
}

void callback_timer_periodic(unsigned long long missed, void *data)
{
    struct ctx_timer *ctxo = data;

    fprintf(stderr, "callback timer periodic called %d\n", ctxo->periodic_runs);

    if (missed > 1)
        fprintf(stderr, "missed timer events: %llu\n", missed - 1);

    ctxo->periodic_runs--;
    if (ctxo->periodic_runs == 0) {
        // finish, enough testing
        int ret = ev_timer_cancel(ctxo->ev, ctxo->eve);
        if (ret < 0) {
            fprintf(stderr, "failed to cancel timer\n");
            exit(EXIT_FAILURE);
        }
        ev_entry_free(ctxo->eve);
        free(ctxo);
    }
}

static void test_timer_periodic(void)
{
    int flags = 0;
    struct timespec ts = {.tv_sec = 1, .tv_nsec = 0};

    fprintf(stderr, "Test: periodic timer\n");

    struct ev *ev = ev_new(0);
    if (!ev) {
        fprintf(stderr, "Cannot create event handler\n");
        return;
    }

    struct ctx_timer *ctxo = ctx_timer_new();
    ctxo->ev = ev;
    ctxo->periodic_runs = 5;

    struct ev_entry *eve =
        ev_timer_periodic_new(&ts, callback_timer_periodic, ctxo);
    if (!eve) {
        fprintf(stderr, "Failed to create a ev_entry object\n");
        exit(EXIT_FAILURE);
    }
    ctxo->eve = eve;

    int ret = ev_add(ev, eve);
    if (ret != 0) {
        fprintf(stderr, "Cannot add entry to event handler\n");
        exit(EXIT_FAILURE);
    }

    // ev_loop will run until the timeout is fired. Which
    // in turn is the last event, which will end the ev loop
    ev_loop(ev, flags);

    ev_destroy(ev);
}

static uint32_t events = EPOLLIN | EPOLLPRI | EPOLLERR | EPOLLHUP;

void fd_cb_raw(int fd, uint32_t events_ret, void *priv_data)
{
    struct ev *ev = priv_data;

    (void) fd;

    assert(events == events_ret);
    ev_run_out(ev);
}

void test_events_raw(void)
{
    struct ev *ev;
    int flags = 0, ret;
    struct ev_entry *eve;
    int pipefd[2];

    fprintf(stderr, "Test: raw events\n");

    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    ev = ev_new(0);
    if (!ev) {
        fprintf(stderr, "Cannot create event handler\n");
        return;
    }

    eve = ev_entry_new_raw(pipefd[0], events, fd_cb_raw, ev);
    if (!eve) {
        fprintf(stderr, "Failed to create a ev_entry object\n");
        exit(EXIT_FAILURE);
    }

    ret = ev_add(ev, eve);
    if (ret != 0) {
        fprintf(stderr, "Cannot add entry to event handler\n");
        exit(EXIT_FAILURE);
    }

    write(pipefd[1], "1", 1);

    // ev_loop will run until the timeout is fired. Which
    // in turn is the last event, which will end the ev loop
    ev_loop(ev, flags);

    ev_entry_free(eve);
    ev_destroy(ev);
}

int main(void)
{
    test_timer_oneshot();
    test_timer_periodic();
    test_timer();
    test_events_raw();

    return EXIT_SUCCESS;
}
