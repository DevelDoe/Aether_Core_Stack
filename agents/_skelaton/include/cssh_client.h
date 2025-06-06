// Project: Seer
// File: include/cssh_client.h

#pragma once

#include <libwebsockets.h>
#include <stdint.h>

int cssh_client_callback(struct lws *wsi,
                         enum lws_callback_reasons reason,
                         void *user,
                         void *in,
                         size_t len);

void start_cssh_client(struct lws_context *context,
                       const char *client_id,
                       const char *name,
                       const char *realm,
                       const char *description,
                       const char *host,
                       const char *cssh_host,
                       int cssh_port);

int cssh_client_is_ready();
uint64_t cssh_client_last_ping_time();
void cssh_client_force_disconnect(void);
void send_log_entry(const char *message, const char *source_id);
