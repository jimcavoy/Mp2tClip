# CMakeList.txt : CMake project for Mp2tClip, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.21)

project(Mp2tClip
	VERSION 1.0.0
	DESCRIPTION "A command line program that decompose (clips) a MPEG-2 TS file/stream into multiple files."
	LANGUAGES CXX
)

include (GNUInstallDirs)

find_package(mp2tp 1 CONFIG REQUIRED)
find_package(h264p 1 CONFIG REQUIRED)

include_directories(../loki-lib/include)

add_executable (Mp2tClip)

# Add source to this project's executable.
file(GLOB SRC_LIST src/*.cpp)
target_sources (Mp2tClip PRIVATE ${SRC_LIST})

if (WIN32)
	set(WS wsock32 ws2_32)
endif()

target_link_libraries(Mp2tClip PRIVATE lcss::mp2tp thetastream::h264p ${WS})

# define the C++ standard needed to compile this library and make it visible to
# dependers
target_compile_features(
    Mp2tClip
    PUBLIC cxx_std_17
)

# Install targets
install(TARGETS Mp2tClip )

# Test cases
enable_testing()

add_test(NAME Clip
	COMMAND Mp2tClip -s ${PROJECT_SOURCE_DIR}/sample/svt_testset_420_720p50_klved_4774.ts -d 10)