// Project: Seer
// File: src/cssh_client.c

#include "cssh_client.h"

#include <json-c/json.h>
#include <libwebsockets.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "logger.h"

// Socket state
static struct lws *client_wsi = NULL;
static int connected = 0;
static int registered = 0;
static const char *reg_id = NULL;
static const char *reg_name = NULL;
static const char *reg_realm = NULL;
static const char *reg_desc = NULL;
static const char *reg_host = NULL;

static uint64_t last_ping_ts = 0;

static uint64_t get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int cssh_client_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            log_info("✅ Connected to CSSH");
            client_wsi = wsi;
            connected = 1;
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (!registered) {
                struct json_object *manifest = json_object_new_object();
                struct json_object *body = json_object_new_object();

                json_object_object_add(body, "id", json_object_new_string(reg_id));
                json_object_object_add(body, "name", json_object_new_string(reg_name));
                json_object_object_add(body, "role", json_object_new_string("agent"));
                json_object_object_add(body, "realm", json_object_new_string(reg_realm));
                json_object_object_add(body, "host", json_object_new_string(reg_host));
                json_object_object_add(body, "description", json_object_new_string(reg_desc));

                json_object_object_add(manifest, "type", json_object_new_string("register"));
                json_object_object_add(manifest, "manifest", body);

                const char *msg = json_object_to_json_string(manifest);
                unsigned char buf[LWS_PRE + 1024];
                size_t msg_len = strlen(msg);
                memcpy(&buf[LWS_PRE], msg, msg_len);

                lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_TEXT);
                json_object_put(manifest);

                log_info("📤 Sent registration manifest: %s", reg_id);
                registered = 1;
            }
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            log_debug("📥 CSSH says: %.*s", (int)len, (char *)in);

            struct json_object *msg = json_tokener_parse(in);
            if (msg) {
                const char *type = json_object_get_string(json_object_object_get(msg, "type"));
                if (type && strcmp(type, "ping") == 0) {
                    last_ping_ts = get_time_ms();

                    struct json_object *pong = json_object_new_object();
                    json_object_object_add(pong, "type", json_object_new_string("pong"));
                    json_object_object_add(pong, "client_id", json_object_new_string(reg_id));

                    const char *json_str = json_object_to_json_string(pong);
                    unsigned char buf[LWS_PRE + 256];
                    size_t msg_len = strlen(json_str);
                    memcpy(&buf[LWS_PRE], json_str, msg_len);
                    lws_write(wsi, &buf[LWS_PRE], msg_len, LWS_WRITE_TEXT);
                    json_object_put(pong);
                }
                json_object_put(msg);
            }
            break;

        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        case LWS_CALLBACK_CLOSED:
            log_info("❌ CSSH disconnected — resetting state");
            connected = 0;
            registered = 0;
            client_wsi = NULL;
            break;

        default:
            break;
    }
    return 0;
}

void start_cssh_client(struct lws_context *context, const char *client_id, const char *name, const char *realm, const char *desc, const char *host, const char *cssh_host, int cssh_port) {
    log_info("🔧 Preparing connection to CSSH with ID: %s\n", client_id);

    // 🛠 Reset state
    connected = 0;
    registered = 0;
    client_wsi = NULL;

    reg_id = client_id;
    reg_name = name;
    reg_realm = realm;
    reg_desc = desc;
    reg_host = host;

    struct lws_client_connect_info ccinfo = {0};
    ccinfo.context = context;
    ccinfo.address = cssh_host;
    ccinfo.port = cssh_port;
    ccinfo.path = "/ws";
    ccinfo.host = cssh_host;
    ccinfo.origin = cssh_host;
    ccinfo.protocol = "cssh-protocol";

    if (!lws_client_connect_via_info(&ccinfo)) {
        log_error("❌ Failed to connect to CSSH at %s:%d", cssh_host, cssh_port);
    } else {
        log_info("🔌 lws_client_connect_via_info() succeeded — waiting for handshake.");
    }
}

int cssh_client_is_ready(void) {
    log_debug("🔍 CSSH ready check — connected: %d, registered: %d", connected, registered);
    return connected && registered;
}

uint64_t cssh_client_last_ping_time(void) {
    uint64_t now = get_time_ms();
    uint64_t age = now - last_ping_ts;

    log_debug("🕒 Last ping: %llu ms ago (timestamp: %llu)", age, last_ping_ts);
    return last_ping_ts;
}

void cssh_client_force_disconnect(void) {
    if (client_wsi) {
        log_info("🔌 Forcing disconnect from CSSH...");
        lws_set_timeout(client_wsi, PENDING_TIMEOUT_CLOSE_SEND, LWS_TO_KILL_ASYNC);
        client_wsi = NULL;
        connected = 0;
        registered = 0;
        last_ping_ts = get_time_ms();  // Reset so we don’t loop
    }
}

void send_log_entry(const char *message, const char *source_id) {
    if (!cssh_client_is_ready()) {
        log_debug("📭 Cannot send log entry — CSSH not ready");
        return;
    }

    struct json_object *msg = json_object_new_object();
    json_object_object_add(msg, "type", json_object_new_string("log"));
    json_object_object_add(msg, "client_id", json_object_new_string(source_id));
    json_object_object_add(msg, "payload", json_object_new_string(message));

    const char *json_str = json_object_to_json_string(msg);
    unsigned char buf[LWS_PRE + 1024];
    size_t len = strlen(json_str);

    memcpy(&buf[LWS_PRE], json_str, len);
    lws_write(client_wsi, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
    json_object_put(msg);

    log_debug("📤 Relayed log to CSSH: %s", message);
}