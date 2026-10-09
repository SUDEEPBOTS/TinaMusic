#pragma once

#include <string>
#include <mutex>

class Logger {
public:
    static void Info(const std::string& scope, const std::string& message);
    static void Warn(const std::string& scope, const std::string& message);
    static void Error(const std::string& scope, const std::string& message);

private:
    static void LogFormatted(const std::string& level, const std::string& scope, const std::string& message);
    static std::string GetCurrentTimestamp();
    static std::mutex log_mutex;
};
