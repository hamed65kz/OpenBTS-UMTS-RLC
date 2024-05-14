function(detect_target_build_arch)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(ARCH "X86" PARENT_SCOPE)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "X64" PARENT_SCOPE)
    else()
        set(ARCH "Unknown" PARENT_SCOPE)
    endif()
endfunction()

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

detect_target_build_arch()
if(ARCH STREQUAL "X86")
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pthreads-w32-2-9-1-release/Pre-built.2/lib/x86)
    message(STATUS "Building Platform is x86")
elseif(ARCH STREQUAL  "X64")
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Externals/pthreads-w32-2-9-1-release/Pre-built.2/lib/x64)
    message(STATUS "Building Platform is x64")
endif()

target_link_libraries(${PROJECT_NAME}  pthreadVC2)
target_link_libraries(${PROJECT_NAME}  Ws2_32)



