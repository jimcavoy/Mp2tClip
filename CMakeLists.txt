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

add_subdirectory (Mp2tClipImpl)

add_executable (Mp2tClip)

# Add source to this project's executable.
target_sources (Mp2tClip PRIVATE src/main.cpp)

target_link_libraries(Mp2tClip PRIVATE Mp2tClipImpl)

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