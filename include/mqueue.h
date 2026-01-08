#ifndef MQUEUE_H
#define MQUEUE_H

#include "common.h"

// Message delivery guarantees
typedef enum {
    DELIVERY_AT_MOST_ONCE,
    DELIVERY_AT_LEAST_ONCE,
    DELIVERY_EXACTLY_ONCE
} delivery_guarantee_t;

// Message ordering
typedef enum {
    ORDERING_NONE,
    ORDERING_FIFO,
    ORDERING_PRIORITY
} ordering_t;

typedef struct mqueue mqueue_t;
typedef struct message message_t;
typedef struct consumer consumer_t;

// Message structure
struct message {
    char* id;
    char* topic;
    void* payload;
    size_t payload_size;
    uint64_t timestamp;
    int priority;
    int retry_count;
    int max_retries;
};

// Message queue functions
mqueue_t* mqueue_create(const char* name, ordering_t ordering);
void mqueue_destroy(mqueue_t* queue);

// Producer functions
int mqueue_publish(mqueue_t* queue, const char* topic, const void* payload, 
                   size_t payload_size);
int mqueue_publish_with_priority(mqueue_t* queue, const char* topic, 
                                  const void* payload, size_t payload_size, int priority);

// Consumer functions
typedef void (*message_handler_t)(const message_t* msg, void* user_data);

consumer_t* mqueue_subscribe(mqueue_t* queue, const char* topic, 
                             message_handler_t handler, void* user_data);
int mqueue_unsubscribe(consumer_t* consumer);
int mqueue_acknowledge(mqueue_t* queue, const char* message_id);
int mqueue_reject(mqueue_t* queue, const char* message_id, bool requeue);

// Queue management
int mqueue_set_delivery_guarantee(mqueue_t* queue, delivery_guarantee_t guarantee);
int mqueue_set_max_retries(mqueue_t* queue, int max_retries);
size_t mqueue_size(mqueue_t* queue);
void mqueue_clear(mqueue_t* queue);

// Statistics
typedef struct {
    size_t total_published;
    size_t total_consumed;
    size_t total_rejected;
    size_t total_retries;
    size_t queue_size;
    size_t consumer_count;
} mqueue_stats_t;

int mqueue_get_stats(mqueue_t* queue, mqueue_stats_t* stats);

#endif // MQUEUE_H
