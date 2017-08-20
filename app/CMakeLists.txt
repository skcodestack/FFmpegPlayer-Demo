

cmake_minimum_required(VERSION 3.4.1)

set(distribution_DIR ../../../../libs )
set(SOURCE_FILES src/main/cpp)
set(INC_DIR_ROOT src/main/cpp/include)
set(INC_DIR src/main/cpp/include/ffmpeg)
set(INC_DIR_YUV src/main/cpp/include/yuv)

include_directories(
                    ${INC_DIR_ROOT}
                    ${INC_DIR}
                    ${INC_DIR}/libavcodec
                    ${INC_DIR}/libavdevice
                    ${INC_DIR}/libavfilter
                    ${INC_DIR}/libavformat
                    ${INC_DIR}/libavutil
                    ${INC_DIR}/libpostproc
                    ${INC_DIR}/libswresample
                    ${INC_DIR}/libswscale
                    ${INC_DIR_YUV}
                    ${INC_DIR_YUV}/libyuv

                    )

find_library(   log-lib
                log )

find_library(graphics jnigraphics)



add_library(    libyuv
                SHARED
                 IMPORTED )


set_target_properties(  libyuv
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libyuv.so)


add_library(    libavutil-54
                SHARED
                 IMPORTED )


set_target_properties(  libavutil-54
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libavutil-54.so)

add_library(    libswresample-1
                SHARED
                 IMPORTED )


set_target_properties(  libswresample-1
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libswresample-1.so)





add_library(    libavcodec-56
                SHARED
                 IMPORTED )
set_target_properties(  libavcodec-56
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libavcodec-56.so)


add_library(    libavformat-56
                SHARED
                 IMPORTED )


set_target_properties(  libavformat-56
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libavformat-56.so)

add_library(    libswscale-3
                SHARED
                 IMPORTED )


set_target_properties(  libswscale-3
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libswscale-3.so)

add_library(    libpostproc-53
                SHARED
                 IMPORTED )


set_target_properties(  libpostproc-53
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libpostproc-53.so)




add_library(    libavfilter-5
                SHARED
                 IMPORTED )


set_target_properties(  libavfilter-5
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libavfilter-5.so)


add_library(    libavdevice-56
                SHARED
                 IMPORTED )
set_target_properties(  libavdevice-56
                        PROPERTIES IMPORTED_LOCATION
                        ${distribution_DIR}/${ANDROID_ABI}/libavdevice-56.so)











set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")




add_library(    myffmpeg
                SHARED
                ${SOURCE_FILES}/stone_ffmpeg_play.cpp
                ${SOURCE_FILES}/av_play.cpp
                ${SOURCE_FILES}/player_queue.cpp)


target_link_libraries(  myffmpeg
                        libavcodec-56
                        libavdevice-56
                        libavfilter-5
                        libavformat-56
                        libavutil-54
                        libpostproc-53
                        libswresample-1
                        libswscale-3

                        libyuv
                        -landroid
                        ${graphics}
                        ${log-lib}
                        )
