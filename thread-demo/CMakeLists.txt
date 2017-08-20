

cmake_minimum_required(VERSION 3.4.1)

set(distribution_DIR ../../../../libs )
set(SOURCE_FILES src/main/cpp)



find_library(   log-lib
                log )



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")




add_library(    pthread
                SHARED
                ${SOURCE_FILES}/posix_pthread.cpp )



target_link_libraries(  pthread
                        -landroid
                        ${log-lib}
                        )
