cmake_minimum_required(VERSION 3.20)
project(YukkiMusicCpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -Wextra -pthread")

include_directories(include /usr/include)

find_package(CURL REQUIRED)

set(SOURCES
    src/logger.cpp
    src/config.cpp
    src/queue_manager.cpp
    src/platform_yt.cpp
    src/audio_engine.cpp
    src/tg_client.cpp
    src/bot_engine.cpp
    src/main.cpp
)

add_executable(yukki-music-bot ${SOURCES})
target_link_libraries(yukki-music-bot PRIVATE CURL::libcurl pthread)

add_executable(test_queue tests/test_queue.cpp src/queue_manager.cpp src/logger.cpp)
target_link_libraries(test_queue PRIVATE pthread)
