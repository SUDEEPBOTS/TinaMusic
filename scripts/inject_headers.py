#!/usr/bin/env python3
import os
import glob
import re

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

BIG_CPP_GO_HEADER = """/*
 * ============================================================================
 *   ████████╗██╗███╗   ██╗ █████╗ ███╗   ███╗██╗   ██╗███████╗██╗ ██████╗
 *   ╚══██╔══╝██║████╗  ██║██╔══██╗████╗ ████║██║   ██║██╔════╝██║██╔════╝
 *      ██║   ██║██╔██╗ ██║███████║██╔████╔██║██║   ██║███████╗██║██║     
 *      ██║   ██║██║╚██╗██║██╔══██║██║╚██╔╝██║██║   ██║╚════██║██║██║     
 *      ██║   ██║██║ ╚████║██║  ██║██║ ╚═╝ ██║╚██████╔╝███████║██║╚██████╗
 *      ╚═╝   ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝
 * ============================================================================
 *
 * Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
 * Architecture: Native C++20 Core + Go MTProto WebRTC Voice Bridge
 * Copyright (C) 2026 TheTeamVivek / SUDEEPBOTS / TinaMusic Contributors
 * All Rights Reserved.
 *
 * ----------------------------------------------------------------------------
 * LEGAL NOTICE & LICENSE (GNU Affero General Public License v3.0):
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
 *
 * STRICT COPYLEFT CLAUSE:
 * In accordance with Section 13 of the GNU AGPLv3, if you modify this program
 * or run it as a network service (e.g., hosted on servers, Telegram, or cloud),
 * you MUST make the complete source code of your modified version available
 * to all users under the exact same AGPLv3 license terms.
 * Closed-source derivatives or rebranded copies are strictly prohibited.
 * ============================================================================
 */

"""

BIG_SH_HEADER = """# ============================================================================
#   ████████╗██╗███╗   ██╗ █████╗ ███╗   ███╗██╗   ██╗███████╗██╗ ██████╗
#   ╚══██╔══╝██║████╗  ██║██╔══██╗████╗ ████║██║   ██║██╔════╝██║██╔════╝
#      ██║   ██║██╔██╗ ██║███████║██╔████╔██║██║   ██║███████╗██║██║     
#      ██║   ██║██║╚██╗██║██╔══██║██║╚██╔╝██║██║   ██║╚════██║██║██║     
#      ██║   ██║██║ ╚████║██║  ██║██║ ╚═╝ ██║╚██████╔╝███████║██║╚██████╗
#      ╚═╝   ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚══════╝╚═╝ ╚═════╝
# ============================================================================
#
# Project: TinaMusic - Telegram's First Native C++20 Music Bot Engine
# Architecture: Native C++20 Core + Go MTProto WebRTC Voice Bridge
# Copyright (C) 2026 TheTeamVivek / SUDEEPBOTS / TinaMusic Contributors
# All Rights Reserved.
#
# ----------------------------------------------------------------------------
# LEGAL NOTICE & LICENSE (GNU Affero General Public License v3.0):
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
# STRICT COPYLEFT CLAUSE:
# In accordance with Section 13 of the GNU AGPLv3, if you modify this program
# or run it as a network service (e.g., hosted on servers, Telegram, or cloud),
# you MUST make the complete source code of your modified version available
# to all users under the exact same AGPLv3 license terms.
# Closed-source derivatives or rebranded copies are strictly prohibited.
# ============================================================================

"""

def inject_cpp_go(filepath):
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()

    # Remove existing header if present
    content = re.sub(r'/\*\s*\n\s*\*\s*============================================================================.*?============================================================================\s*\*/\s*\n*', '', content, flags=re.DOTALL)
    
    new_content = BIG_CPP_GO_HEADER + content.lstrip()
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(new_content)
    print(f"Injected big AGPLv3 header: {filepath}")

def inject_sh(filepath):
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()

    shebang = "#!/bin/bash\n"
    if content.startswith("#!/bin/bash"):
        content = content[len(shebang):]

    # Remove existing header if present
    content = re.sub(r'# ============================================================================.*?# ============================================================================\s*\n*', '', content, flags=re.DOTALL)
    
    new_content = shebang + BIG_SH_HEADER + content.lstrip()
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(new_content)
    print(f"Injected big AGPLv3 header: {filepath}")

def main():
    cpp_files = glob.glob(os.path.join(BASE_DIR, "src", "**", "*.cpp"), recursive=True) + \
                glob.glob(os.path.join(BASE_DIR, "include", "**", "*.hpp"), recursive=True) + \
                glob.glob(os.path.join(BASE_DIR, "tests", "**", "*.cpp"), recursive=True)
    
    go_files = glob.glob(os.path.join(BASE_DIR, "bridge", "**", "*.go"), recursive=True)
    
    sh_files = [
        os.path.join(BASE_DIR, "install.sh"),
        os.path.join(BASE_DIR, "start.sh"),
    ]
    
    for f in cpp_files:
        inject_cpp_go(f)
        
    for f in go_files:
        inject_cpp_go(f)
        
    for f in sh_files:
        if os.path.exists(f):
            inject_sh(f)

if __name__ == "__main__":
    main()
