// Project: Seer
// File: src/main.c

#include <libwebsockets.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cssh_client.h"
#include "logger.h"
#include "manifest.h"
#include "utils.h"

#define MANIFEST_PATH "/opt/seer/manifest.json"

static volatile sig_atomic_t interrupted = 0;
static volatile sig_atomic_t reload_requested = 0;
static volatile int connected = 0;

void sigint_handler(int sig) {
    (void)sig;
    interrupted = 1;
}

void sighup_handler(int sig) {
    (void)sig;
    reload_requested = 1;
}

static struct lws_protocols protocols[] = {{
                                               .name = "cssh-protocol",
                                               .callback = cssh_client_callback,
                                               .rx_buffer_size = 2048,
                                           },
                                           {NULL, NULL, 0, 0}};

int main(void) {
    signal(SIGINT, sigint_handler);
    signal(SIGHUP, sighup_handler);  // Soft reload support

    // 📦 Load manifest once
    Manifest manifest = load_manifest(MANIFEST_PATH);
    logger_set_id(manifest.id);
    logger_init_file(manifest.name);
    logger_set_email_config("smtp.office365.com", "alerts@yourdomain.com", "you@yourdomain.com");
    logger_enable_email(1);

    log_info("📦 Loaded manifest: %s (%s)", manifest.name, manifest.id);

    // 🌐 WebSocket setup
    struct lws_context_creation_info info = {0};
    struct lws_context *context;

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context) {
        log_error("❌ Failed to create WebSocket context");
        return 1;
    }

    // 🚀 Initial connection
    start_cssh_client(context, manifest.id, manifest.name, manifest.realm, manifest.description, manifest.host, "127.0.0.1", 8111);

    static int forced_disconnect = 0;

    while (!interrupted) {
        lws_service(context, 1000);  // 🔁 process LWS events

        uint64_t now = get_time_ms();
        uint64_t last_ping = cssh_client_last_ping_time();

        if (cssh_client_is_ready() && last_ping > 0 && now - last_ping > 15000 && !forced_disconnect) {
            log_warn("⚠️ No ping from CSSH for 15s — forcing disconnect");
            cssh_client_force_disconnect();
            forced_disconnect = 1;
        }

        if (!cssh_client_is_ready()) {
            forced_disconnect = 0;
            log_info("🔁 CSSH not ready — attempting reconnect...");
            start_cssh_client(context, manifest.id, manifest.name, manifest.realm, manifest.description, manifest.host, "127.0.0.1", 8111);
            sleep(3);
            continue;
        }

        if (reload_requested) {
            reload_requested = 0;
            log_info("🔄 Received SIGHUP — reloading log file");
            logger_reopen_file();
        }
    }

    lws_context_destroy(context);
    return 0;
}
