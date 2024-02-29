cmake_minimum_required(VERSION 3.21.0)

project (
    Mp2tClipImpl
    VERSION 1.0.0
    DESCRIPTION "The core code for the Mp2tClip application."
    LANGUAGES CXX
)

find_package(mp2tp 1 CONFIG REQUIRED)
find_package(h264p 1 CONFIG REQUIRED)

include_directories(../../loki-lib/include)

add_library(Mp2tClipImpl STATIC)

file(GLOB SRC_LIST src/*.cpp)

target_sources(Mp2tClipImpl PRIVATE ${SRC_LIST})

set_property(TARGET Mp2tClipImpl PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_features(Mp2tClipImpl PUBLIC cxx_std_17)

if(WIN32)
    set(WS wsock32 ws2_32)
endif()

target_link_libraries(Mp2tClipImpl lcss::mp2tp thetastream::h264p ${WS})

target_include_directories(
    Mp2tClipImpl
    PRIVATE src/Mp2tClipImpl 
    PUBLIC include
)