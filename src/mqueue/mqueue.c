#define _POSIX_C_SOURCE 200809L
#include "mqueue.h"
#include <pthread.h>

#define MAX_MESSAGE_ID 64

// Queue node
typedef struct queue_node {
    message_t* message;
    struct queue_node* next;
} queue_node_t;

// Consumer structure
struct consumer {
    char* topic;
    message_handler_t handler;
    void* user_data;
    pthread_t thread;
    int active;
    mqueue_t* queue;
    struct consumer* next;
};

// Message queue structure
struct mqueue {
    char* name;
    ordering_t ordering;
    delivery_guarantee_t guarantee;
    int max_retries;
    
    queue_node_t* head;
    queue_node_t* tail;
    size_t size;
    
    consumer_t* consumers;
    size_t consumer_count;
    
    pthread_mutex_t lock;
    pthread_cond_t cond;
    
    // Statistics
    size_t total_published;
    size_t total_consumed;
    size_t total_rejected;
    size_t total_retries;
    
    int running;
};

static char* generate_message_id(void) {
    static uint64_t counter = 0;
    char* id = safe_malloc(MAX_MESSAGE_ID);
    snprintf(id, MAX_MESSAGE_ID, "msg-%lu-%lu", get_timestamp_ms(), counter++);
    return id;
}

static message_t* message_create(const char* topic, const void* payload, 
                                 size_t payload_size, int priority) {
    message_t* msg = safe_calloc(1, sizeof(message_t));
    msg->id = generate_message_id();
    msg->topic = safe_strdup(topic);
    msg->payload = safe_malloc(payload_size);
    memcpy(msg->payload, payload, payload_size);
    msg->payload_size = payload_size;
    msg->timestamp = get_timestamp_ms();
    msg->priority = priority;
    msg->retry_count = 0;
    msg->max_retries = 3;
    return msg;
}

static void message_destroy(message_t* msg) {
    if (!msg) return;
    safe_free((void**)&msg->id);
    safe_free((void**)&msg->topic);
    safe_free((void**)&msg->payload);
    safe_free((void**)&msg);
}

static void enqueue_message(mqueue_t* queue, queue_node_t* node) {
    if (queue->ordering == ORDERING_PRIORITY) {
        // Insert based on priority
        queue_node_t* current = queue->head;
        queue_node_t* prev = NULL;
        
        while (current && current->message->priority >= node->message->priority) {
            prev = current;
            current = current->next;
        }
        
        node->next = current;
        if (prev) {
            prev->next = node;
        } else {
            queue->head = node;
        }
        
        if (!node->next) {
            queue->tail = node;
        }
    } else {
        // FIFO or none
        node->next = NULL;
        if (queue->tail) {
            queue->tail->next = node;
        } else {
            queue->head = node;
        }
        queue->tail = node;
    }
    
    queue->size++;
}

static queue_node_t* dequeue_message(mqueue_t* queue) {
    if (!queue->head) return NULL;
    
    queue_node_t* node = queue->head;
    queue->head = node->next;
    
    if (!queue->head) {
        queue->tail = NULL;
    }
    
    queue->size--;
    return node;
}

static void* consumer_thread(void* arg) {
    consumer_t* consumer = (consumer_t*)arg;
    mqueue_t* queue = consumer->queue;
    
    while (consumer->active) {
        pthread_mutex_lock(&queue->lock);
        
        while (queue->size == 0 && consumer->active) {
            pthread_cond_wait(&queue->cond, &queue->lock);
        }
        
        if (!consumer->active) {
            pthread_mutex_unlock(&queue->lock);
            break;
        }
        
        queue_node_t* node = dequeue_message(queue);
        
        pthread_mutex_unlock(&queue->lock);
        
        if (node && node->message) {
            // Check if topic matches
            if (strcmp(node->message->topic, consumer->topic) == 0) {
                consumer->handler(node->message, consumer->user_data);
                queue->total_consumed++;
            } else {
                // Re-queue if topic doesn't match
                pthread_mutex_lock(&queue->lock);
                enqueue_message(queue, node);
                pthread_mutex_unlock(&queue->lock);
                continue;
            }
            
            message_destroy(node->message);
            safe_free((void**)&node);
        }
    }
    
    return NULL;
}

mqueue_t* mqueue_create(const char* name, ordering_t ordering) {
    mqueue_t* queue = safe_calloc(1, sizeof(mqueue_t));
    queue->name = safe_strdup(name);
    queue->ordering = ordering;
    queue->guarantee = DELIVERY_AT_LEAST_ONCE;
    queue->max_retries = 3;
    queue->running = 1;
    
    pthread_mutex_init(&queue->lock, NULL);
    pthread_cond_init(&queue->cond, NULL);
    
    return queue;
}

void mqueue_destroy(mqueue_t* queue) {
    if (!queue) return;
    
    queue->running = 0;
    
    // Stop all consumers
    consumer_t* consumer = queue->consumers;
    while (consumer) {
        consumer_t* next = consumer->next;
        mqueue_unsubscribe(consumer);
        consumer = next;
    }
    
    // Clear queue
    mqueue_clear(queue);
    
    safe_free((void**)&queue->name);
    pthread_mutex_destroy(&queue->lock);
    pthread_cond_destroy(&queue->cond);
    safe_free((void**)&queue);
}

int mqueue_publish(mqueue_t* queue, const char* topic, const void* payload, 
                   size_t payload_size) {
    return mqueue_publish_with_priority(queue, topic, payload, payload_size, 0);
}

int mqueue_publish_with_priority(mqueue_t* queue, const char* topic, 
                                  const void* payload, size_t payload_size, int priority) {
    if (!queue || !topic || !payload || payload_size == 0) {
        return ERROR_INVALID_PARAM;
    }
    
    message_t* msg = message_create(topic, payload, payload_size, priority);
    msg->max_retries = queue->max_retries;
    
    queue_node_t* node = safe_calloc(1, sizeof(queue_node_t));
    node->message = msg;
    
    pthread_mutex_lock(&queue->lock);
    enqueue_message(queue, node);
    queue->total_published++;
    pthread_cond_broadcast(&queue->cond);
    pthread_mutex_unlock(&queue->lock);
    
    return SUCCESS;
}

consumer_t* mqueue_subscribe(mqueue_t* queue, const char* topic, 
                             message_handler_t handler, void* user_data) {
    if (!queue || !topic || !handler) {
        return NULL;
    }
    
    consumer_t* consumer = safe_calloc(1, sizeof(consumer_t));
    consumer->topic = safe_strdup(topic);
    consumer->handler = handler;
    consumer->user_data = user_data;
    consumer->queue = queue;
    consumer->active = 1;
    
    pthread_mutex_lock(&queue->lock);
    consumer->next = queue->consumers;
    queue->consumers = consumer;
    queue->consumer_count++;
    pthread_mutex_unlock(&queue->lock);
    
    pthread_create(&consumer->thread, NULL, consumer_thread, consumer);
    
    return consumer;
}

int mqueue_unsubscribe(consumer_t* consumer) {
    if (!consumer) return ERROR_INVALID_PARAM;
    
    consumer->active = 0;
    mqueue_t* queue = consumer->queue;
    
    pthread_mutex_lock(&queue->lock);
    pthread_cond_broadcast(&queue->cond);
    pthread_mutex_unlock(&queue->lock);
    
    pthread_join(consumer->thread, NULL);
    
    // Remove from queue's consumer list
    pthread_mutex_lock(&queue->lock);
    consumer_t* current = queue->consumers;
    consumer_t* prev = NULL;
    
    while (current) {
        if (current == consumer) {
            if (prev) {
                prev->next = current->next;
            } else {
                queue->consumers = current->next;
            }
            queue->consumer_count--;
            break;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&queue->lock);
    
    safe_free((void**)&consumer->topic);
    safe_free((void**)&consumer);
    
    return SUCCESS;
}

int mqueue_acknowledge(mqueue_t* queue, const char* message_id) {
    // Placeholder for acknowledgment logic
    (void)queue;
    (void)message_id;
    return SUCCESS;
}

int mqueue_reject(mqueue_t* queue, const char* message_id, bool requeue) {
    if (!queue || !message_id) {
        return ERROR_INVALID_PARAM;
    }
    
    (void)requeue;  // Placeholder for requeue logic
    
    pthread_mutex_lock(&queue->lock);
    queue->total_rejected++;
    pthread_mutex_unlock(&queue->lock);
    
    return SUCCESS;
}

int mqueue_set_delivery_guarantee(mqueue_t* queue, delivery_guarantee_t guarantee) {
    if (!queue) return ERROR_INVALID_PARAM;
    queue->guarantee = guarantee;
    return SUCCESS;
}

int mqueue_set_max_retries(mqueue_t* queue, int max_retries) {
    if (!queue) return ERROR_INVALID_PARAM;
    queue->max_retries = max_retries;
    return SUCCESS;
}

size_t mqueue_size(mqueue_t* queue) {
    if (!queue) return 0;
    
    pthread_mutex_lock(&queue->lock);
    size_t size = queue->size;
    pthread_mutex_unlock(&queue->lock);
    
    return size;
}

void mqueue_clear(mqueue_t* queue) {
    if (!queue) return;
    
    pthread_mutex_lock(&queue->lock);
    
    queue_node_t* current = queue->head;
    while (current) {
        queue_node_t* next = current->next;
        message_destroy(current->message);
        safe_free((void**)&current);
        current = next;
    }
    
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    
    pthread_mutex_unlock(&queue->lock);
}

int mqueue_get_stats(mqueue_t* queue, mqueue_stats_t* stats) {
    if (!queue || !stats) {
        return ERROR_INVALID_PARAM;
    }
    
    pthread_mutex_lock(&queue->lock);
    
    stats->total_published = queue->total_published;
    stats->total_consumed = queue->total_consumed;
    stats->total_rejected = queue->total_rejected;
    stats->total_retries = queue->total_retries;
    stats->queue_size = queue->size;
    stats->consumer_count = queue->consumer_count;
    
    pthread_mutex_unlock(&queue->lock);
    
    return SUCCESS;
}
