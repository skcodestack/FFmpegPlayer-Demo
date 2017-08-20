cmake_minimum_required(VERSION 3.4.1)

set(distribution_DIR ../../../../libs )
set(SOURCE_FILES src/main/cpp)
set(INC_DIR_ROOT src/main/cpp/include)

include_directories(

                    ${INC_DIR_ROOT}
                    ${INC_DIR_ROOT}/x264
                    ${INC_DIR_ROOT}/faac
                    ${INC_DIR_ROOT}/rtmpdump
                    )

find_library(   log-lib
                log )

find_library(graphics jnigraphics)

add_library(    libx264
                STATIC
                IMPORTED )


set_target_properties(  libx264
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libx264.a)

add_library(    libfaac
                STATIC
                IMPORTED )


set_target_properties(  libfaac
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libfaac.a)

add_library(    librtmp
                STATIC
                IMPORTED )


set_target_properties(  librtmp
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/librtmp.a)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")


add_library(    mylive
                SHARED
                ${SOURCE_FILES}/mylive.cpp
                ${SOURCE_FILES}/queue.cpp
                )



target_link_libraries(  mylive
                        libx264
                        libfaac
                        librtmp
                        -landroid
                        ${graphics}
                        ${log-lib}
                        )

