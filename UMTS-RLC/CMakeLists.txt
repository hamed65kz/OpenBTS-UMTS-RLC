cmake_minimum_required(VERSION 3.28.0)
project(UMTSRLCLib VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED On)
set(CMAKE_CXX_EXTENSIONS Off)


include(Utilities/utilities.cmake)
include(Ue/ue.cmake)
include(RLC/rlc.cmake)
include(MAC/mac.cmake)
include(RRC/rrc.cmake)

add_library(${PROJECT_NAME} STATIC URlc.cpp ${SOURCES})

#for URlc.h being visible by module caller
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pthreads-w32-2-9-1-release/Pre-built.2/include )
target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86)


target_link_libraries(${PROJECT_NAME}  pthreadVC2)
target_link_libraries(${PROJECT_NAME}  Ws2_32)



