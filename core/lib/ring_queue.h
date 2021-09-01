/**
 * \file ringbuf_queue.h         \brief Ring buffer queue template
 *
 *
 * Template usage example:
 * ------------------------
 *
 * #define QUEUE_TYPE uint16_t
 * #define QUEUE_NAME(x) EVENTS_##x
 * #define QUEUE_FUNC(x) events_##x
 * #define QUEUE_SIZE   8
 * #include "ringbuf_queue.h"
 *
 */


typedef struct
{
    QUEUE_TYPE items[QUEUE_SIZE];
    uint8_t start;
    uint8_t count;

} QUEUE_NAME(queue_t);


static inline void QUEUE_FUNC(queue_init)(QUEUE_NAME(queue_t) *q)
{
    q->start = 0;
    q->count = 0;
}

/** Get first packet from queue */
static inline QUEUE_TYPE *QUEUE_FUNC(queue_front)(QUEUE_NAME(queue_t) *q)
{
    return &q->items[q->start];
}

/** Push item to queue */
static inline void QUEUE_FUNC(queue_push)(QUEUE_NAME(queue_t) *q, QUEUE_TYPE *item)
{
    int end = (q->start + q->count) % QUEUE_SIZE;

    q->items[end] = *item;
    if (q->count == QUEUE_SIZE)
        q->start = (q->start + 1) % QUEUE_SIZE; // full, overwrite
    else
        q->count++;
}

/** Get pointer to next item in the queue */
static inline QUEUE_TYPE *QUEUE_FUNC(queue_push_empty)(QUEUE_NAME(queue_t) *q)
{
    int end;

    if (q->count == QUEUE_SIZE)
        q->start = (q->start + 1) % QUEUE_SIZE; // full, overwrite
    else
        q->count++;

    end = (q->start + q->count) % QUEUE_SIZE;

    return &q->items[end];
}

/** Get item from queue */
static inline QUEUE_TYPE *QUEUE_FUNC(queue_pop)(QUEUE_NAME(queue_t) *q)
{
    QUEUE_TYPE *item = NULL;

    if (q->count > 0)
    {
        item = &q->items[q->start];
        q->start = (q->start + 1) % QUEUE_SIZE;
        q->count--;
    }

    return item;
}

/** Get item from queue without remove it */
static inline QUEUE_TYPE *QUEUE_FUNC(queue_peek)(QUEUE_NAME(queue_t) *q)
{
    return (q->count > 0) ? &q->items[q->start] : NULL;
}

/** Get items count in the queue */
static inline uint16_t QUEUE_FUNC(queue_count)(QUEUE_NAME(queue_t) *q)
{
    return q->count;
}

