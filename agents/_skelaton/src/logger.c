// Project: Seer
// File: src/logger.c

#include "logger.h"

#include <ctype.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cssh_client.h"

static LogLevel current_level = LOG_LEVEL_INFO;

// file
static const char *logger_id = NULL;
static FILE *log_file = NULL;
static char log_path[128] = {0};

// email
static int email_enabled = 0;
static char smtp_host[128] = {0};
static char sender_email[128] = {0};
static char recipient_email[128] = {0};

typedef struct {
    LogLevel level;
    char message[1024];
} EmailJob;

void logger_set_id(const char *id) { logger_id = id; }
void log_set_level(LogLevel level) { current_level = level; }

// file
void logger_init_file(const char *name) {
    char sanitized[64] = {0};
    size_t j = 0;

    for (size_t i = 0; name[i] && j < sizeof(sanitized) - 1; ++i) {
        char c = name[i];
        if (isalnum(c) || c == '_' || c == '-') {
            sanitized[j++] = c;
        } else if (c == ' ') {
            sanitized[j++] = '_';
        }
    }

    snprintf(log_path, sizeof(log_path), "/var/log/%s.log", sanitized);
    log_file = fopen(log_path, "a");

    if (!log_file) {
        fprintf(stderr, "[%s] [ERROR] Failed to open log file: %s\n", logger_id, log_path);
    } else {
        fprintf(stderr, "[%s] [INFO] Logging to file: %s\n", logger_id, log_path);
    }
}

void logger_reopen_file(void) {
    if (!log_path[0]) return;

    FILE *new_file = freopen(log_path, "a", log_file);
    if (!new_file) {
        fprintf(stderr, "[%s] [ERROR] Failed to reopen log file: %s\n", logger_id, log_path);
    } else {
        log_file = new_file;
        fprintf(stderr, "[%s] [INFO] Log file reopened: %s\n", logger_id, log_path);
    }
}

// email
void logger_set_email_config(const char *smtp, const char *sender, const char *recipient) {
    strncpy(smtp_host, smtp, sizeof(smtp_host) - 1);
    strncpy(sender_email, sender, sizeof(sender_email) - 1);
    strncpy(recipient_email, recipient, sizeof(recipient_email) - 1);
}

void logger_enable_email(int enabled) { email_enabled = enabled; }

void *email_thread_func(void *arg) {
    EmailJob *job = (EmailJob *)arg;

    FILE *mail = popen("/usr/sbin/sendmail -t", "w");
    if (!mail) {
        fprintf(stderr, "[%s] [WARN] Failed to spawn sendmail\n", logger_id ? logger_id : "unknown");
        free(job);
        return NULL;
    }

    fprintf(mail, "To: %s\n", recipient_email);
    fprintf(mail, "From: %s\n", sender_email);
    fprintf(mail, "Subject: [CSSH] %s alert from %s\n", job->level == LOG_LEVEL_CRITICAL ? "CRITICAL" : job->level == LOG_LEVEL_ERROR ? "ERROR" : "WARNING", logger_id ? logger_id : "unknown");
    fprintf(mail, "\n%s\n", job->message);
    pclose(mail);

    free(job);
    return NULL;
}

static void send_log_email(LogLevel level, const char *message) {
    if (!email_enabled || !recipient_email[0] || !sender_email[0]) return;

    EmailJob *job = malloc(sizeof(EmailJob));
    if (!job) return;

    job->level = level;
    strncpy(job->message, message, sizeof(job->message) - 1);
    job->message[sizeof(job->message) - 1] = '\0';

    pthread_t tid;
    if (pthread_create(&tid, NULL, email_thread_func, job) == 0) {
        pthread_detach(tid);  // 🔥 fire-and-forget
    } else {
        free(job);
    }
}

void log_msg(LogLevel level, const char *fmt, ...) {
    if (level > current_level) return;

    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    const char *id = logger_id ? logger_id : "unknown";

    // Print to stderr
    fprintf(stderr, "[%s] %s\n", id, buffer);

    // Write to log file
    if (log_file) {
        fprintf(log_file, "[%s] %s\n", id, buffer);
        fflush(log_file);
    }

    // Broadcast everything except debug
    if (level <= LOG_LEVEL_INFO) {
        send_log_entry(buffer, logger_id);
    }

    // Email alerts for warnings and above
    if (level <= LOG_LEVEL_WARN) {
        send_log_email(level, buffer);
    }
}