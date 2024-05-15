
cmake_minimum_required(VERSION 3.28.0)
project(UMTS-RLC-APP VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)

add_executable(${PROJECT_NAME} UMTS-RLC.cpp ${SOURCES})

include(../UMTS-RLC/CMake/utilities.cmake)
set(WIN_PTHREAD_PREBUILD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pthreads-w32-2-9-1-release/Pre-built.2)
add_pthread(${PROJECT_NAME} ${WIN_PTHREAD_PREBUILD_PATH})


target_link_libraries(${PROJECT_NAME}  UMTSRLCLib)

