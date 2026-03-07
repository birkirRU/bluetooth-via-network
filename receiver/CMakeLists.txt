cmake_minimum_required(VERSION 3.15)
project(audio_receiver C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(OPUS REQUIRED opus)
else()
    set(OPUS_INCLUDE_DIRS "/usr/include")
    set(OPUS_LIBRARIES "/usr/lib/x86_64-linux-gnu/libopus.so")
endif()

include_directories(${OPUS_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.c
    src/udp_receiver.c
    src/jitter_buffer.c
    src/opus_decoder.c
)

set(HEADERS
    include/packet.h
    include/jitter_buffer.h
    include/opus_decoder.h
    include/udp_receiver.h
)

add_executable(audio_receiver ${SOURCES} ${HEADERS})

target_link_libraries(audio_receiver
    ${OPUS_LIBRARIES}
    pthread
)

target_compile_options(audio_receiver PRIVATE -Wall -Wextra)
