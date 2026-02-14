#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "../common/definitions.h"

struct Block;

enum LogLevel {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
    LOG_SUCCESS
};

void init_logger(const std::string& filename = "debug.log");
void close_logger();

void log_message(LogLevel level, const std::string& message);
void log_debug(const std::string& message);
void log_info(const std::string& message);
void log_warning(const std::string& message);
void log_error(const std::string& message);
void log_success(const std::string& message);

void set_console_output(bool enabled);
void set_file_output(bool enabled);
void set_log_level(LogLevel level);

void clear_log();
void log_separator();
void log_block_info(const Block* block, const std::string& prefix = "");

#endif
