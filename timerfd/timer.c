#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

struct posix_timer {
    int tfd;
    struct itimerspec ts;
    timer_t timer;
};

static int efd;

static void do_handle(struct posix_timer *pt)
{
    int tfd = pt->tfd;

    pt->ts.it_value.tv_sec = 1;
    pt->ts.it_value.tv_nsec = 0;
    pt->ts.it_interval.tv_sec = 0;
    pt->ts.it_interval.tv_nsec = 0;

    timerfd_settime(tfd, 0, &pt->ts, NULL);
}

static void start_epoll(int fd, void *data)
{
    efd = epoll_create(1);

    struct epoll_event ev = {.events = EPOLLIN, .data.ptr = data};
    if (epoll_ctl(efd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        perror("epoll_ctl");
        exit(1);
    }

    while (1) {
        printf("start to epoll wait...\n");
        struct epoll_event events;
        int ret = epoll_wait(efd, &events, 1, -1);
        if (ret < 0) {
            perror("epoll_wait");
            exit(1);
        }

        printf("read...\n");
        struct posix_timer *pt = events.data.ptr;

        /* events is overwritten by epoll_wait, and have ev.data.fd */
        int buf;
        read(pt->tfd, &buf, sizeof(uint64_t));
        printf("[%s] ret %d buf %d \n", __FUNCTION__, ret, buf);
        do_handle(pt);
    }
    exit(1);
}

int main()
{
    struct posix_timer pt = {.tfd = timerfd_create(CLOCK_REALTIME, 0)};
    if (pt.tfd < 0) {
        perror("timerfd_create");
        exit(1);
    }

    pt.ts.it_value.tv_sec = 5;
    pt.ts.it_value.tv_nsec = 0;
    pt.ts.it_interval.tv_sec = 0;
    pt.ts.it_interval.tv_nsec = 0;

    // run timer with rel-timer mode.
    if (timerfd_settime(pt.tfd, 0, &pt.ts, NULL)) {
        perror("timerfd_settime");
        exit(1);
    }

    if (!fork())
        start_epoll(pt.tfd, &pt);

    while (1) {
        puts(".");
        sleep(3);
    }
    return 0;
}
