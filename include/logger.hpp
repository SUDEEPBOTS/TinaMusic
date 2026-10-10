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
