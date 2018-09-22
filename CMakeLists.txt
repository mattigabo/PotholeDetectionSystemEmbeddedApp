cmake_minimum_required(VERSION 3.10)
project(PotholeDetectionObserver)
set(PROJECT_VERSION 0.0.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Where to find CMake modules and OpenCV
if ("$ENV{OPENCV}" STREQUAL "")
    message("ERROR: The OPENCV environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{RAPIDJSON}" STREQUAL "")
    message("ERROR: The RAPIDJSON environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{PhD}" STREQUAL "")
    message("ERROR: The PhD environmental/global variable is not set or is empty")
    return(0)
else ()
    message("$ENV{OPENCV}")
    message("$ENV{RAPIDJSON}")
    message("$ENV{PhD}")

    # Init OPENCV Environment
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules")

    set(OPENCV_DIR $ENV{OPENCV}/install)
    set(OPENCV_INCLUDE ${OPENCV_DIR}/include)
    set(OPENCV_LIBS ${OPENCV_DIR}/x64/mingw/lib)
    set(OPENCV_BIN ${OPENCV_DIR}/x64/mingw/bin)
    set(OPENCV_LIB_WORLD ${OPENCV_LIBS}/libopencv_world340.dll.a)

    set(PHD_LIBS $ENV{PhD}/lib/static)
    set(PHD_BIN $ENV{PhD}/bin)
    set(PHD_LIB_WORLD ${PHD_LIBS}/libphd_world.dll.a)
    set(PHD_INCLUDE $ENV{PhD}/include)

    set(RAPIDJSON_INCLUDE $ENV{RAPIDJSON}/include)

    find_package(OPENCV REQUIRED)

    include_directories(
            ${OPENCV_INCLUDE}
            ${RAPIDJSON_INCLUDE}
            ${PHD_INCLUDE}
            ${PROJECT_SOURCE_DIR}/include
    )

    add_executable(PotholeDetectionObserver main.cpp)
    # linking

    target_link_libraries(
            PotholeDetectionObserver
            ${PHD_LIB_WORLD}
            ${OPENCV_LIB_WORLD}
    )

    install(
            TARGETS PotholeDetectionObserver
            ARCHIVE         DESTINATION         "lib/static"
            LIBRARY         DESTINATION         "lib/shared"
            RUNTIME         DESTINATION         "bin"
            PUBLIC_HEADER   DESTINATION         "include/phdo"
    )

    return(1)
endif ()