#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include "common.h"

// ============================================================================
// WebSocket Protocol Implementation (RFC 6455)
// ============================================================================

typedef enum {
    WS_OPCODE_CONTINUATION = 0x0,
    WS_OPCODE_TEXT = 0x1,
    WS_OPCODE_BINARY = 0x2,
    WS_OPCODE_CLOSE = 0x8,
    WS_OPCODE_PING = 0x9,
    WS_OPCODE_PONG = 0xA
} ws_opcode_t;

typedef enum {
    WS_STATE_CONNECTING,
    WS_STATE_OPEN,
    WS_STATE_CLOSING,
    WS_STATE_CLOSED
} ws_state_t;

typedef struct ws_frame ws_frame_t;
typedef struct ws_connection ws_connection_t;
typedef struct ws_server ws_server_t;

// WebSocket frame
struct ws_frame {
    int fin;
    ws_opcode_t opcode;
    int masked;
    uint64_t payload_length;
    uint8_t mask_key[4];
    void* payload;
};

// WebSocket callbacks
typedef void (*ws_on_connect_cb)(ws_connection_t* conn, void* user_data);
typedef void (*ws_on_message_cb)(ws_connection_t* conn, const void* data, 
                                size_t length, int is_binary, void* user_data);
typedef void (*ws_on_close_cb)(ws_connection_t* conn, int code, 
                              const char* reason, void* user_data);
typedef void (*ws_on_error_cb)(ws_connection_t* conn, int error_code, void* user_data);

// WebSocket Server operations
ws_server_t* ws_server_create(int port);
void ws_server_destroy(ws_server_t* server);
int ws_server_start(ws_server_t* server);
int ws_server_stop(ws_server_t* server);
int ws_server_set_on_connect(ws_server_t* server, ws_on_connect_cb callback, 
                             void* user_data);
int ws_server_set_on_message(ws_server_t* server, ws_on_message_cb callback,
                             void* user_data);
int ws_server_set_on_close(ws_server_t* server, ws_on_close_cb callback,
                           void* user_data);
int ws_server_set_on_error(ws_server_t* server, ws_on_error_cb callback,
                           void* user_data);

// WebSocket Connection operations
int ws_connection_send_text(ws_connection_t* conn, const char* message);
int ws_connection_send_binary(ws_connection_t* conn, const void* data, size_t length);
int ws_connection_ping(ws_connection_t* conn);
int ws_connection_pong(ws_connection_t* conn);
int ws_connection_close(ws_connection_t* conn, int code, const char* reason);
ws_state_t ws_connection_get_state(const ws_connection_t* conn);
const char* ws_connection_get_id(const ws_connection_t* conn);

// WebSocket Frame operations
ws_frame_t* ws_frame_create(ws_opcode_t opcode, const void* payload, 
                           size_t payload_length);
void ws_frame_destroy(ws_frame_t* frame);
int ws_frame_encode(const ws_frame_t* frame, void* buffer, size_t* buffer_len);
ws_frame_t* ws_frame_decode(const void* buffer, size_t buffer_len);

// ============================================================================
// Real-time Communication Patterns
// ============================================================================

// Publish-Subscribe pattern for WebSocket
typedef struct ws_pubsub ws_pubsub_t;

ws_pubsub_t* ws_pubsub_create(void);
void ws_pubsub_destroy(ws_pubsub_t* pubsub);
int ws_pubsub_subscribe(ws_pubsub_t* pubsub, ws_connection_t* conn, 
                       const char* topic);
int ws_pubsub_unsubscribe(ws_pubsub_t* pubsub, ws_connection_t* conn, 
                         const char* topic);
int ws_pubsub_publish(ws_pubsub_t* pubsub, const char* topic, 
                     const void* data, size_t length);
int ws_pubsub_publish_text(ws_pubsub_t* pubsub, const char* topic, 
                          const char* message);

// Broadcasting
int ws_server_broadcast_text(ws_server_t* server, const char* message);
int ws_server_broadcast_binary(ws_server_t* server, const void* data, size_t length);

// Room-based communication
typedef struct ws_room ws_room_t;

ws_room_t* ws_room_create(const char* room_name);
void ws_room_destroy(ws_room_t* room);
int ws_room_join(ws_room_t* room, ws_connection_t* conn);
int ws_room_leave(ws_room_t* room, ws_connection_t* conn);
int ws_room_broadcast(ws_room_t* room, const void* data, size_t length);
size_t ws_room_get_member_count(const ws_room_t* room);

// ============================================================================
// Server-Sent Events (SSE)
// ============================================================================

typedef struct sse_connection sse_connection_t;
typedef struct sse_server sse_server_t;

// SSE Server operations
sse_server_t* sse_server_create(void);
void sse_server_destroy(sse_server_t* server);
sse_connection_t* sse_server_accept_connection(sse_server_t* server, 
                                               const char* client_id);
int sse_server_send_event(sse_server_t* server, const char* client_id,
                         const char* event_type, const char* data);
int sse_server_broadcast_event(sse_server_t* server, const char* event_type,
                               const char* data);

// SSE Connection operations
int sse_connection_send(sse_connection_t* conn, const char* event_type, 
                       const char* data);
int sse_connection_close(sse_connection_t* conn);
void sse_connection_destroy(sse_connection_t* conn);

// ============================================================================
// Long Polling
// ============================================================================

typedef struct long_poll_manager long_poll_manager_t;
typedef struct long_poll_request long_poll_request_t;

long_poll_manager_t* long_poll_manager_create(uint64_t timeout_ms);
void long_poll_manager_destroy(long_poll_manager_t* manager);
long_poll_request_t* long_poll_request_create(const char* client_id);
int long_poll_request_wait(long_poll_request_t* request, void** data, 
                           size_t* data_len);
int long_poll_manager_push_data(long_poll_manager_t* manager, 
                                const char* client_id,
                                const void* data, size_t data_len);
void long_poll_request_destroy(long_poll_request_t* request);

#endif // WEBSOCKET_H
