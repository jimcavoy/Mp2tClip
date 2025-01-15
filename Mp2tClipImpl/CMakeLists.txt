cmake_minimum_required(VERSION 3.21.0)

project (
    Mp2tClipImpl
    VERSION 1.0.1
    DESCRIPTION "The core code for the Mp2tClip application."
    LANGUAGES CXX
)

find_package(mp2tp 1 CONFIG REQUIRED)
find_package(h264p 1 CONFIG REQUIRED)

# include the module `FetchContent`
include(FetchContent)

# Add an external project from a downloaded source archive
FetchContent_Declare(loki-lib
  URL https://github.com/snaewe/loki-lib/archive/refs/tags/Release_0_1_5.tar.gz
  URL_HASH MD5=74e60c683f745dc15c6e772927349483
)
FetchContent_MakeAvailable(loki-lib)
FetchContent_GetProperties(loki-lib)

set(LOKI_DIR "${loki-lib_SOURCE_DIR}")

add_library(Mp2tClipImpl STATIC)

target_sources(Mp2tClipImpl
  PRIVATE
    include/Mp2tClipImpl/Clipper.h
    include/Mp2tClipImpl/CommandLineParser.h
    include/Mp2tClipImpl/Monitor.h
    src/AccessUnit.cpp
    src/AccessUnit.h
    src/Clipper.cpp
    src/CommandLineParser.cpp
    src/Monitor.cpp
    src/Mpeg2TsDecoder.cpp
    src/Mpeg2TsDecoder.h
    src/PCRClock.cpp
    src/PCRClock.h
    src/PmtProxy.cpp
    src/PmtProxy.h
    src/VideoDecoder.cpp
    src/VideoDecoder.h
)

set_property(TARGET Mp2tClipImpl PROPERTY POSITION_INDEPENDENT_CODE ON)

target_compile_features(Mp2tClipImpl PUBLIC cxx_std_17)

if(WIN32)
    target_link_libraries(Mp2tClipImpl lcss::mp2tp thetastream::h264p wsock32 ws2_32)
else()
    target_link_libraries(Mp2tClipImpl lcss::mp2tp thetastream::h264p)
endif()



target_include_directories( Mp2tClipImpl
  PRIVATE 
    src/Mp2tClipImpl
  PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${LOKI_DIR}/include>
)
