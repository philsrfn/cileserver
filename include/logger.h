#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} log_level_t;

/**
 * Initialize the logger
 * 
 * @return 0 on success, non-zero on failure
 */
int init_logger(void);

/**
 * Clean up logger resources
 * 
 * @return 0 on success, non-zero on failure
 */
int cleanup_logger(void);

/**
 * Set the log level
 * 
 * @param level The minimum log level to output
 */
void set_log_level(log_level_t level);

/**
 * Log a debug message
 * 
 * @param format Format string
 * @param ... Variable arguments
 */
void log_debug(const char *format, ...);

/**
 * Log an info message
 * 
 * @param format Format string
 * @param ... Variable arguments
 */
void log_info(const char *format, ...);

/**
 * Log a warning message
 * 
 * @param format Format string
 * @param ... Variable arguments
 */
void log_warning(const char *format, ...);

/**
 * Log an error message
 * 
 * @param format Format string
 * @param ... Variable arguments
 */
void log_error(const char *format, ...);

#endif /* LOGGER_H */ 
