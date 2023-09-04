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

if (CMAKE_VS_PLATFORM_NAME)
    include_directories(../../../../lib/mp2tp/libmp2t)
    add_library(lcss::libmp2t STATIC IMPORTED)
    set_target_properties(lcss::libmp2t PROPERTIES
                 IMPORTED_LOCATION ../../../../lib/mp2tp/libmp2t/Release/libmp2t.lib
                 IMPORTED_LOCATION_DEBUG ../../../../lib/mp2tp/libmp2t/Debug/libmp2t.lib
                 IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
    include_directories(../../../../lib/h264p/libh264p)
    add_library(ThetaStream::libh264p STATIC IMPORTED)
    set_target_properties(ThetaStream::libh264p PROPERTIES
                 IMPORTED_LOCATION ../../../../lib/h264p/libh264p/Release/libh264p.lib
                 IMPORTED_LOCATION_DEBUG ../../../../lib/h264p/libh264p/Debug/libh264p.lib
                 IMPORTED_CONFIGURATIONS "RELEASE;DEBUG")
    add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
else ()
    include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libmp2t/libmp2tTargets.cmake)
    include(${CMAKE_INSTALL_LIBDIR}/lib/cmake/libh264p/libh264pTargets.cmake)
endif()

# Add source to this project's executable.
file(GLOB SRC_LIST
    *.h
    *.cpp)
add_executable (Mp2tClip ${SRC_LIST})

list(APPEND EXTRA_LIBS lcss::libmp2t ThetaStream::libh264p wsock32 ws2_32)

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