#include "logger.h"
#include "../common/definitions.h"
#include "file_io.h"
#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace {
    std::ofstream logFile;
    bool consoleEnabled = true;
    bool fileEnabled = true;
    LogLevel currentMinLevel = LOG_DEBUG;
    std::string currentLogFile = "";
}


std::string get_timestamp() {
    time_t now = time(nullptr);
    tm* timeinfo = localtime(&now);
    
    std::stringstream ss;
    ss << std::setfill('0') 
       << std::setw(2) << timeinfo->tm_hour << ":"
       << std::setw(2) << timeinfo->tm_min << ":"
       << std::setw(2) << timeinfo->tm_sec;
    
    return ss.str();
}

std::string get_color_code(LogLevel level) {
    switch(level) {
        case LOG_DEBUG:   return "\033[36m"; // Cyan
        case LOG_INFO:    return "\033[37m"; // White
        case LOG_WARNING: return "\033[33m"; // Yellow
        case LOG_ERROR:   return "\033[31m"; // Red
        case LOG_SUCCESS: return "\033[32m"; // Green
        default:          return "\033[0m";  // Reset
    }
}

std::string get_level_string(LogLevel level) {
    switch(level) {
        case LOG_DEBUG:
            return "[DEBUG]  ";
        case LOG_INFO:
            return "[INFO]   ";
        case LOG_WARNING:
            return "[WARNING]";
        case LOG_ERROR:
            return "[ERROR]  ";
        case LOG_SUCCESS:
            return "[SUCCESS]";
        default:
            return "[UNKNOWN]";
    }
}

void init_logger(const std::string& filename) {
    currentLogFile = filename;
    logFile.open(filename, std::ios::app);
    
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file: " << filename << std::endl;
        fileEnabled = false;
    } else {
        logFile << "\n\n========================================\n";
        logFile << "Log Session Started at " << get_timestamp() << "\n";
        logFile << "========================================\n\n";
    }
}


void close_logger() {
    if (logFile.is_open()) {
        logFile << "\n========================================\n";
        logFile << "Log Session Ended at " << get_timestamp() << "\n";
        logFile << "========================================\n";
        logFile.close();
    }
}

void log_message(LogLevel level, const std::string& message) {
    if (level < currentMinLevel) { // eg: messages should not be logged when app is runnung at release
        return;
    }
    
    std::string timestamp = get_timestamp();
    std::string levelStr = get_level_string(level);
    std::string fullMessage = timestamp + " " + levelStr + " " + message;
    
    if (consoleEnabled) {
        std::cout << get_color_code(level) 
                  << fullMessage 
                  << "\033[0m" << std::endl;
    }
    
    if (fileEnabled && logFile.is_open()) {
        logFile << fullMessage << std::endl;
        logFile.flush();
    }
}

void log_debug(const std::string& message) {
    log_message(LOG_DEBUG, message);
}

void log_info(const std::string& message) {
    log_message(LOG_INFO, message);
}

void log_warning(const std::string& message) {
    log_message(LOG_WARNING, message);
}

void log_error(const std::string& message) {
    log_message(LOG_ERROR, message);
}

void log_success(const std::string& message) {
    log_message(LOG_SUCCESS, message);
}

void set_console_output(bool enabled) {
    consoleEnabled = enabled;
}

void set_file_output(bool enabled) {
    fileEnabled = enabled;
}

void set_log_level(LogLevel minLevel) {
    currentMinLevel = minLevel;
}

void clear_log() {
    if (logFile.is_open()) {
        logFile.close();
    }
    
    logFile.open(currentLogFile, std::ios::trunc);
    
    if (logFile.is_open()) {
        logFile << "========================================\n";
        logFile << "Log Cleared at " << get_timestamp() << "\n";
        logFile << "========================================\n\n";
    }
}

void log_separator() {
    log_message(LOG_INFO, "----------------------------------------");
}

void log_block_info(const Block* block, const std::string& prefix) {
    if (block == nullptr) {
        log_warning(prefix + "Block is NULL");
        return;
    }
    
    std::stringstream ss;
    ss << prefix << "Block #" << block->id 
       << " [Type: " << blocktype_to_string(block->type) << "]"
       << " Pos: (" << block->x << ", " << block->y << ")"
       << " Size: " << block->width << "x" << block->height;
    
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
    
    if (block->next) {
        log_debug(prefix + "  Next -> Block #" + std::to_string(block->next->id));
    }
    
    if (block->inner) {
        log_debug(prefix + "  Inner -> Block #" + std::to_string(block->inner->id));
    }
}
