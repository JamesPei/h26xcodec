cmake_minimum_required(VERSION 3.17)
project(h26xcodec)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

find_package(FFMPEG REQUIRED COMPONENTS avcodec avutil swscale avformat)
find_package(CXXOPTS REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRCS)

add_executable(${PROJECT_NAME} ${SRCS})
target_include_directories(${PROJECT_NAME} PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> 
    )
target_link_libraries(${PROJECT_NAME} PRIVATE FFMPEG::avcodec FFMPEG::avutil FFMPEG::avformat FFMPEG::swscale x264 x265 pthread z swresample m vdpau X11 va va-drm va-x11 nlohmann_json::nlohmann_json)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/local/bin)
