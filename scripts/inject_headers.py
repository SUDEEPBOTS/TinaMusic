#!/usr/bin/env python3
import os
import glob

BASE_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

CPP_GO_HEADER = """/*
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

"""

SH_HEADER = """# ============================================================================
# TinaMusic - Telegram's First Native C++20 Music Engine
# Copyright (C) 2026 TheTeamVivek / TinaMusic Contributors
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
# ============================================================================

"""

def inject_cpp_go(filepath):
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()
    if "TinaMusic - Telegram's First Native C++20 Music Engine" in content:
        print(f"Skipping (already injected): {filepath}")
        return
    new_content = CPP_GO_HEADER + content
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(new_content)
    print(f"Injected header into: {filepath}")

def inject_sh(filepath):
    with open(filepath, "r", encoding="utf-8") as f:
        lines = f.readlines()
    content = "".join(lines)
    if "TinaMusic - Telegram's First Native C++20 Music Engine" in content:
        print(f"Skipping (already injected): {filepath}")
        return
    if lines and lines[0].startswith("#!"):
        new_content = lines[0] + "\n" + SH_HEADER + "".join(lines[1:])
    else:
        new_content = SH_HEADER + content
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(new_content)
    print(f"Injected header into shell script: {filepath}")

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
