cmake_minimum_required(VERSION 3.15)
project(AudioStreamClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(OPUS REQUIRED opus)
elseif(WIN32)
    find_path(OPUS_INCLUDE_DIRS NAMES opus/opus.h PATHS 
        "C:/msys64/ucrt64/include"
        "C:/msys64/mingw64/include"
        "C:/vcpkg/installed/x64-mingw-dynamic/include"
        "C:/opus/include")
    find_library(OPUS_LIBRARY NAMES opus libopus PATHS 
        "C:/msys64/ucrt64/lib"
        "C:/msys64/mingw64/lib"
        "C:/vcpkg/installed/x64-mingw-dynamic/lib"
        "C:/opus/lib")
    if(OPUS_INCLUDE_DIRS AND OPUS_LIBRARY)
        set(OPUS_FOUND TRUE)
        set(OPUS_INCLUDE_DIRS "${OPUS_INCLUDE_DIRS}")
        set(OPUS_LIBRARIES "${OPUS_LIBRARY}")
    else()
        message(FATAL_ERROR "Opus not found. Install with: pacman -S mingw-w64-ucrt-x86_64-opus")
    endif()
else()
    set(OPUS_INCLUDE_DIRS "C:/msys64/ucrt64/include")
    set(OPUS_LIBRARY_DIRS "C:/msys64/ucrt64/lib")
    set(OPUS_LIBRARIES "opus")
endif()

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

message(STATUS "Opus include: ${OPUS_INCLUDE_DIRS}")
message(STATUS "Opus libs: ${OPUS_LIBRARIES}")
