#ifndef queue_h_
#define queue_h_

#include <pthread.h>
#include <stdint.h>
#include <linux/memfd.h>

typedef struct {
    // backing buffer and size
    uint8_t *buffer;
    size_t size;

    // backing buffer's memfd descriptor
    int fd;

    // read / write indices
    size_t head, tail;

    // sequence number of next consumable message
    size_t head_seq;

    // sequence number of last written message
    size_t tail_seq;

    // synchronization primitives
    pthread_cond_t readable, writeable;
    pthread_mutex_t lock;
} queue_t;

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/types.h>

/* Metadata (header) for a message in the queue */
typedef struct {
    size_t len, seq;
} message_t;

/* Convenience wrappers for erroring out */
static inline void queue_error(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "queue error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}
static inline void queue_error_errno(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "queue error: ");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, " (errno %d)\n", errno);
    va_end(args);
    abort();
}

/** Initialize a blocking queue *q* of size *s* */
void queue_init(queue_t *q, size_t s)
{
    /* We mmap two adjacent pages (in virtual memory) that point to the same
     * physical memory. This lets us optimize memory access, so that we don't
     * need to even worry about wrapping our pointers around until we go
     * through the entire buffer.
     */

    // Check that the requested size is a multiple of a page. If it isn't, we're
    // in trouble.
    if (s % getpagesize() != 0) {
        queue_error(
            "Requested size (%lu) is not a multiple of the page size (%d)", s,
            getpagesize());
    }

    // Create an anonymous file backed by memory
    if ((q->fd = memfd_create("queue_region", 0)) == -1)
        queue_error_errno("Could not obtain anonymous file");

    // Set buffer size
    if (ftruncate(q->fd, s) != 0)
        queue_error_errno("Could not set size of anonymous file");

    // Ask mmap for a good address
    if ((q->buffer = mmap(NULL, 2 * s, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS,
                          -1, 0)) == MAP_FAILED)
        queue_error_errno("Could not allocate virtual memory");

    // Mmap first region
    if (mmap(q->buffer, s, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
             q->fd, 0) == MAP_FAILED)
        queue_error_errno("Could not map buffer into virtual memory");

    // Mmap second region, with exact address
    if (mmap(q->buffer + s, s, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED,
             q->fd, 0) == MAP_FAILED)
        queue_error_errno("Could not map buffer into virtual memory");

    // Initialize synchronization primitives
    if (pthread_mutex_init(&q->lock, NULL) != 0)
        queue_error_errno("Could not initialize mutex");
    if (pthread_cond_init(&q->readable, NULL) != 0)
        queue_error_errno("Could not initialize condition variable");
    if (pthread_cond_init(&q->writeable, NULL) != 0)
        queue_error_errno("Could not initialize condition variable");

    // Initialize remaining members
    q->size = s;
    q->head = q->tail = 0;
    q->head_seq = q->tail_seq = 0;
}

/** Destroy the blocking queue *q* */
void queue_destroy(queue_t *q)
{
    if (munmap(q->buffer + q->size, q->size) != 0)
        queue_error_errno("Could not unmap buffer");

    if (munmap(q->buffer, q->size) != 0)
        queue_error_errno("Could not unmap buffer");

    if (close(q->fd) != 0)
        queue_error_errno("Could not close anonymous file");

    if (pthread_mutex_destroy(&q->lock) != 0)
        queue_error_errno("Could not destroy mutex");

    if (pthread_cond_destroy(&q->readable) != 0)
        queue_error_errno("Could not destroy condition variable");

    if (pthread_cond_destroy(&q->writeable) != 0)
        queue_error_errno("Could not destroy condition variable");
}

/** Insert into queue *q* a message of *size* bytes from *buffer*
 *
 * Blocks until sufficient space is available in the queue.
 */
void queue_put(queue_t *q, uint8_t *buffer, size_t size)
{
    pthread_mutex_lock(&q->lock);

    // Wait for space to become available
    while ((q->size - (q->tail - q->head)) < (size + sizeof(message_t)))
        pthread_cond_wait(&q->writeable, &q->lock);

    // Construct header
    message_t m = {.len = size, .seq = q->tail_seq++};

    // Write message
    memcpy(&q->buffer[q->tail], &m, sizeof(message_t));
    memcpy(&q->buffer[q->tail + sizeof(m)], buffer, size);

    // Increment write index
    q->tail += sizeof(m) + size;

    pthread_cond_signal(&q->readable);
    pthread_mutex_unlock(&q->lock);
}

/** Retrieves a message of at most *max* bytes from queue *q* and writes
 * it to *buffer*.
 *
 * Blocks until a message of no more than *max* bytes is available.
 *
 * Returns the number of bytes in the written message.
 */
size_t queue_get(queue_t *q, uint8_t *buffer, size_t max)
{
    pthread_mutex_lock(&q->lock);

    // Wait for a message that we can successfully consume to reach the front of
    // the queue
    message_t m;
    for (;;) {
        // Wait for a message to arrive
        while ((q->tail - q->head) == 0)
            pthread_cond_wait(&q->readable, &q->lock);

        // Read message header
        memcpy(&m, &q->buffer[q->head], sizeof(message_t));

        // Message too long, wait for someone else to consume it
        if (m.len > max) {
            while (q->head_seq == m.seq)
                pthread_cond_wait(&q->writeable, &q->lock);
            continue;
        }

        // We successfully consumed the header of a suitable message, so proceed
        break;
    }

    // Read message body
    memcpy(buffer, &q->buffer[q->head + sizeof(message_t)], m.len);

    // Consume the message by incrementing the read pointer
    q->head += m.len + sizeof(message_t);
    q->head_seq++;

    // When read buffer moves into 2nd memory region, we can reset to the 1st
    // region
    if (q->head >= q->size) {
        q->head -= q->size;
        q->tail -= q->size;
    }

    pthread_cond_signal(&q->writeable);
    pthread_mutex_unlock(&q->lock);

    return m.len;
}

#endif
