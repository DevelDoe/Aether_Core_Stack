// Project: Seer
// File: include/logger.h

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

typedef enum {
    LOG_LEVEL_CRITICAL = 0,  // 🔥 Highest priority
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARN,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG
} LogLevel;

void log_set_level(LogLevel level);
void logger_set_id(const char *id);

// File logging
void logger_init_file(const char *name);
void logger_reopen_file(void);

// New: Email integration
void logger_set_email_config(const char *smtp_host, const char *sender, const char *recipient);
void logger_enable_email(int enabled);

// Main log function
void log_msg(LogLevel level, const char *fmt, ...);

// Convenience macros
#define log_critical(fmt, ...) log_msg(LOG_LEVEL_CRITICAL, "[CRITICAL] " fmt, ##__VA_ARGS__)
#define log_error(fmt, ...)    log_msg(LOG_LEVEL_ERROR,    "[ERROR] " fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...)     log_msg(LOG_LEVEL_WARN,     "[WARN] "  fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)     log_msg(LOG_LEVEL_INFO,     "[INFO] "  fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...)    log_msg(LOG_LEVEL_DEBUG,    "[DEBUG] " fmt, ##__VA_ARGS__)

#endif