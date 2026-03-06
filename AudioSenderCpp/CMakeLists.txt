cmake_minimum_required(VERSION 3.15)
project(AudioStreamClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(OPUS REQUIRED opus)

include_directories(${OPUS_INCLUDE_DIRS})
link_directories(${OPUS_LIBRARY_DIRS})

set(SOURCES
    main.cpp
    audio_capture.cpp
    opus_encoder.cpp
    network_sender.cpp
)

set(HEADERS
    audio_capture.h
    opus_encoder.h
    network_sender.h
)

add_executable(AudioStreamClient ${SOURCES} ${HEADERS})

target_link_libraries(AudioStreamClient 
    ${OPUS_LIBRARIES}
    ws2_32
    ole32
    uuid
    winmm
)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(AudioStreamClient PRIVATE /W3 /EHsc)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(AudioStreamClient PRIVATE -Wall -Wextra)
endif()

target_compile_definitions(AudioStreamClient PRIVATE
    SERVER_IP="${SERVER_IP}"
    SERVER_PORT=${SERVER_PORT}
)

message(STATUS "Opus include: ${OPUS_INCLUDE_DIRS}")
message(STATUS "Opus libs: ${OPUS_LIBRARIES}")
