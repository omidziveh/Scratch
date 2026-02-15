#include "logger.h"
#include "../common/definitions.h"
#include "../backend/file_io.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <SDL2/SDL.h>
#include <cstring>

static std::ofstream logFile;
static bool consoleEnabled = true;
static bool fileEnabled = true;
static LogLevel minLogLevel = LOG_DEBUG;
static char write_buffer[8192];
static int buffer_pos = 0;
static Uint32 last_flush_time = 0;
static const int FLUSH_INTERVAL_MS = 2000;
static const int BUFFER_THRESHOLD = 6144;  
static void flush_buffer() {
    if (buffer_pos > 0 && logFile.is_open()) {
        logFile.write(write_buffer, buffer_pos);
        logFile.flush();
        buffer_pos = 0;
    }
    last_flush_time = SDL_GetTicks();
}

static void buffer_write(const char* data, int len) {
    if (buffer_pos + len >= (int)sizeof(write_buffer))
        flush_buffer();

    if (len >= (int)sizeof(write_buffer)) {
        if (logFile.is_open()) {
            logFile.write(data, len);
            logFile.flush();
        }
        return;
    }

    memcpy(write_buffer + buffer_pos, data, len);
    buffer_pos += len;

    if (buffer_pos >= BUFFER_THRESHOLD)
        flush_buffer();
}

void init_logger(const std::string& filename) {
    logFile.open(filename, std::ios::out | std::ios::app);
    if (logFile.is_open()) {
        log_separator();
        log_info("Logger initialized");
    }
}

void close_logger() {
    if (logFile.is_open()) {
        log_info("Logger closing");
        flush_buffer();
        logFile.close();
    }
}

static std::string get_timestamp() {
    time_t now = time(nullptr);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return std::string(buf);
}

static std::string level_to_string(LogLevel level) {
    switch (level) {
        case LOG_DEBUG:   return "DEBUG";
        case LOG_INFO:    return "INFO";
        case LOG_WARNING: return "WARN";
        case LOG_ERROR:   return "ERROR";
        case LOG_SUCCESS: return "OK";
    }
    return "???";
}
void log_message(LogLevel level, const std::string& message) {
    if (level < minLogLevel) return;

    std::string line = "[" + get_timestamp() + "] [" + level_to_string(level) + "] " + message + "\n";

    if (consoleEnabled) {
        std::cout << line;
    }

    if (fileEnabled && logFile.is_open()) {
        buffer_write(line.c_str(), (int)line.size());
        if (level >= LOG_ERROR)
            flush_buffer();
    }
}


void log_debug(const std::string& message)   { log_message(LOG_DEBUG, message); }
void log_info(const std::string& message)    { log_message(LOG_INFO, message); }
void log_warning(const std::string& message) { log_message(LOG_WARNING, message); }
void log_error(const std::string& message)   { log_message(LOG_ERROR, message); }
void log_success(const std::string& message) { log_message(LOG_SUCCESS, message); }

void set_console_output(bool enabled) { consoleEnabled = enabled; }
void set_file_output(bool enabled)    { fileEnabled = enabled; }
void set_log_level(LogLevel level)    { minLogLevel = level; }

void clear_log() {
    if (logFile.is_open()) {
        logFile.close();
        logFile.open("debug.log", std::ios::out | std::ios::trunc);
    }
}

void log_separator() {
    std::string sep(60, '=');
    log_message(LOG_INFO, sep);
}
void log_block_info(const Block* block, const std::string& prefix) {
    if (!block) {
        log_warning(prefix + "Block is NULL");
        return;
    }

    std::stringstream ss;
    ss << prefix << "Block #" << block->id
       << " [" << blocktype_to_string(block->type) << "]"
       << " at (" << block->x << ", " << block->y << ")";
    log_info(ss.str());

    if (!block->args.empty()) {
        ss.str("");
        ss << prefix << "  Args: [";
        for (size_t i = 0; i < block->args.size(); i++) {
            ss << block->args[i];
            if (i < block->args.size() - 1) ss << ", ";
        }
        ss << "]";
        log_debug(ss.str());
    }

    if (block->parent) {
        log_debug(prefix + "  Parent: #" + std::to_string(block->parent->id));
    }
    if (block->next) {
        log_debug(prefix + "  Child: #" + std::to_string(block->next->id));
    }
}
void logger_tick() {
    Uint32 now = SDL_GetTicks();
    if (now - last_flush_time >= (Uint32)FLUSH_INTERVAL_MS)
        flush_buffer();
}
