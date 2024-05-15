function(detect_target_build_arch)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(ARCH "X86" PARENT_SCOPE)
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(ARCH "X64" PARENT_SCOPE)
    else()
        set(ARCH "Unknown" PARENT_SCOPE)
    endif()
endfunction()


function(add_pthread target_name)
    set(PTHREAD_PREBUILD_PATH_WIN ${ARGV1}) # it is optional param, should set on Windows
    #PTHREAD_PREBUILD_PATH_WIN is like "root/pthreads-w32-2-9-1-release/Pre-built.2" it is download package in Windows
    
    if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
        #sudo apt install libpthread-stubs0-dev 
        find_package(Threads REQUIRED)
        target_link_libraries(${target_name} Threads::Threads)
        
    elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")   
        target_include_directories(${target_name} PRIVATE ${PTHREAD_PREBUILD_PATH_WIN}/include)    
        message(STATUS "inc : " ${PTHREAD_PREBUILD_PATH_WIN}/include)   
        detect_target_build_arch()
        if(ARCH STREQUAL "X86")
            target_link_directories(${target_name} PRIVATE ${PTHREAD_PREBUILD_PATH_WIN}/lib/x86)
        elseif(ARCH STREQUAL  "X64")
            target_link_directories(${target_name} PRIVATE ${PTHREAD_PREBUILD_PATH_WIN}/lib/x64)
        endif() 
        target_link_libraries(${target_name} pthreadVC2)
    endif()
endfunction()