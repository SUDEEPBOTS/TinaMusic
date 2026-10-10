/*
 * ============================================================================
 * TinaMusic - Telegram's First Native C++20 Music Engine
 * Copyright (C) 2026 TheTeamVivek / TinaMusic Contributors
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 * ============================================================================
 */

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
