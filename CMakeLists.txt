cmake_minimum_required(VERSION 3.8)

set(CURRENT_PROJECT_NAME audio-player)
project(${CURRENT_PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")

file(GLOB HEADERS
  "*.h"
  "*.hh")
file(GLOB SOURCES
  "*.c"
  "*.cpp"
  "*.cc")

add_library(${CURRENT_PROJECT_NAME}
  SHARED
  ${HEADERS}
  ${SOURCES})

target_link_libraries(${CURRENT_PROJECT_NAME}
  portaudio
  portaudiocpp
  sndfile)

