#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>
#include "../include/logger.h"

#define LOG_FILENAME "logs/cileserver.log"
#define MAX_LOG_SIZE 1024

static FILE *log_file = NULL;
static log_level_t current_log_level = LOG_INFO;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

static const char *level_strings[] = {
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"
};

int init_logger(void) {
    pthread_mutex_lock(&log_mutex);
    
    if (log_file != NULL) {
        pthread_mutex_unlock(&log_mutex);
        return 0;  // Already initialized
    }
    
    log_file = fopen(LOG_FILENAME, "a");
    if (log_file == NULL) {
        pthread_mutex_unlock(&log_mutex);
        return -1;
    }
    
    // Write initialization message
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[26];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(log_file, "[%s] [INFO] Logger initialized\n", timestamp);
    fflush(log_file);
    
    pthread_mutex_unlock(&log_mutex);
    return 0;
}

int cleanup_logger(void) {
    pthread_mutex_lock(&log_mutex);
    
    if (log_file != NULL) {
        // Write cleanup message
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char timestamp[26];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
        
        fprintf(log_file, "[%s] [INFO] Logger shutting down\n", timestamp);
        fflush(log_file);
        
        fclose(log_file);
        log_file = NULL;
    }
    
    pthread_mutex_unlock(&log_mutex);
    return 0;
}

void set_log_level(log_level_t level) {
    pthread_mutex_lock(&log_mutex);
    current_log_level = level;
    pthread_mutex_unlock(&log_mutex);
}

static void log_message(log_level_t level, const char *format, va_list args) {
    if (level < current_log_level) {
        return;
    }
    
    pthread_mutex_lock(&log_mutex);
    
    if (log_file == NULL) {
        // Try to initialize if not already done
        if (init_logger() != 0) {
            pthread_mutex_unlock(&log_mutex);
            return;
        }
    }
    
    // Get current time
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[26];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    // Format the message
    char message[MAX_LOG_SIZE];
    vsnprintf(message, sizeof(message), format, args);
    
    // Write to log file
    fprintf(log_file, "[%s] [%s] %s\n", timestamp, level_strings[level], message);
    fflush(log_file);
    
    // Also print to stderr for errors
    if (level >= LOG_WARNING) {
        fprintf(stderr, "[%s] [%s] %s\n", timestamp, level_strings[level], message);
    }
    
    pthread_mutex_unlock(&log_mutex);
}

void log_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_DEBUG, format, args);
    va_end(args);
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_INFO, format, args);
    va_end(args);
}

void log_warning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_WARNING, format, args);
    va_end(args);
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    log_message(LOG_ERROR, format, args);
    va_end(args);
}
