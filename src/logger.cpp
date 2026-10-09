#include "logger.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

std::mutex Logger::log_mutex;

std::string Logger::GetCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void Logger::LogFormatted(const std::string& level, const std::string& scope, const std::string& message) {
    std::lock_guard<std::mutex> lock(log_mutex);
    std::cout << "[" << GetCurrentTimestamp() << "] [" << level << "] [" << scope << "] " << message << std::endl;
}

void Logger::Info(const std::string& scope, const std::string& message) {
    LogFormatted("INFO", scope, message);
}

void Logger::Warn(const std::string& scope, const std::string& message) {
    LogFormatted("WARN", scope, message);
}

void Logger::Error(const std::string& scope, const std::string& message) {
    LogFormatted("ERROR", scope, message);
}
