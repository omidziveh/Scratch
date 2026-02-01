#pragma once
#include <string>

// Log levels
enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_SUCCESS
};

// Initialize logger
void init_logger(const std::string& filename = "debug.log");

// Close logger
void close_logger();

// Main logging function
void log_message(LogLevel level, const std::string& message);

// Convenience functions
void log_debug(const std::string& message);
void log_info(const std::string& message);
void log_warning(const std::string& message);
void log_error(const std::string& message);
void log_success(const std::string& message);

// Enable/disable console output
void set_console_output(bool enabled);

// Enable/disable file output
void set_file_output(bool enabled);

// Set minimum log level
void set_log_level(LogLevel minLevel);

// Clear log file
void clear_log();

// Print separator line
void log_separator();

// Log block information
void log_block_info(const struct Block* block, const std::string& prefix = "");
