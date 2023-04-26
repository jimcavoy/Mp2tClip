# CMakeList.txt : CMake project for Mp2tClip, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
project(Mp2tClip)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (WIN32)
    set(CMAKE_INSTALL_LIBDIR $ENV{APPDATA})
    include_directories(../../loki-lib/include)
else()
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
    include_directories($ENV{HOME}/.vs/loki-lib/include)
endif()

include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libmp2t/libmp2tTargets.cmake)
include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libh264p/libh264pTargets.cmake)

# Add source to this project's executable.
add_executable (Mp2tClip "main.cpp" "CommandLineParser.cpp" "Mpeg2TsDecoder.cpp" "PCRClock.cpp" "PmtProxy.cpp" "VideoDecoder.cpp" "AccessUnit.cpp")

list(APPEND EXTRA_LIBS lcss::libmp2t ThetaStream::libh264p)

target_link_libraries(Mp2tClip PUBLIC ${EXTRA_LIBS})

# Install targets
install(TARGETS Mp2tClip 
    EXPORT Mp2tClipTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Test cases
enable_testing()

add_test(NAME Clip
	COMMAND Mp2tClip -s${PROJECT_SOURCE_DIR}/../sample/svt_testset_420_720p50_klved_4774.ts -d10)