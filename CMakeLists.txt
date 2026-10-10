cmake_minimum_required(VERSION 3.20)
project(TinaMusic VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -pthread")

include_directories(include vendor/ntgcalls/include /usr/include)
link_directories(vendor/ntgcalls/lib)

find_package(CURL REQUIRED)

set(SOURCES
    src/logger.cpp
    src/config.cpp
    src/queue_manager.cpp
    src/platform_yt.cpp
    src/platforms.cpp
    src/database.cpp
    src/locales.cpp
    src/audio_engine.cpp
    src/voice_bridge.cpp
    src/tg_client.cpp
    src/bot_engine.cpp
    src/main.cpp
)

add_executable(tina-music-bot ${SOURCES})
target_link_libraries(tina-music-bot PRIVATE CURL::libcurl ntgcalls yaml-cpp pthread dl z resolv)

add_executable(test_full tests/test_full.cpp src/queue_manager.cpp src/logger.cpp src/database.cpp src/locales.cpp)
target_link_libraries(test_full PRIVATE yaml-cpp pthread)
