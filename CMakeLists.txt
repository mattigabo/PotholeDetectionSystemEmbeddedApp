cmake_minimum_required(VERSION 3.10)
project(PotholeDetectionObserver)
set(PROJECT_VERSION 0.0.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Where to find CMake modules and OpenCV
if ("$ENV{OpenCV_DIR}" STREQUAL "")
    message("ERROR: The OPENCV environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{RapidJSON_DIR}" STREQUAL "")
    message("ERROR: The RAPIDJSON environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{PhD_DIR}" STREQUAL "")
    message("ERROR: The PhD environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{cURL_DIR}" STREQUAL "")
    message("ERROR: The cURL environmental/global variable is not set or is empty")
    return(0)
else ()
    message("$ENV{OpenCV_DIR}")
    message("$ENV{RapidJSON_DIR}")
    message("$ENV{PhD_DIR}")
    message("$ENV{cURL_DIR}")

    # Init OPENCV Environment
#    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules")

    set(PHD_INCLUDE $ENV{PhD_DIR}/include)
    set(cURL_INCLUDE $ENV{cURL_DIR}/include)

    file(GLOB PHD_LIBS "$ENV{PhD_DIR}/lib/static/lib*")
    file(GLOB cURL_LIBS "$ENV{cURL_DIR}/lib/lib*")

    set(RAPIDJSON_INCLUDE $ENV{RapidJSON_DIR}/include)

    message(Found Libraries: ${PHD_LIBRARIES})

    find_package(OpenCV REQUIRED)
    find_package(cURL REQUIRED)
#    add_subdirectory(lib/curlcpp)

    include_directories(
            ${OpenCV_INCLUDE_DIRS}
            ${RAPIDJSON_INCLUDE}
            ${PHD_INCLUDE}
            ${PROJECT_SOURCE_DIR}/include
            ${cURL_INCLUDE}
#            ${CURLCPP_SOURCE_DIR}/include
    )

    add_executable(
            PotholeDetectionObserver main.cpp
            src/gps.cpp include/iot/gps.h
            src/camera.cpp include/iot/camera.h
            src/networking.cpp include/iot/networking.h
        )

    # linking
    target_link_libraries(
            PotholeDetectionObserver
            ${PHD_LIBS}
            ${OpenCV_LIBS}
            ${cURL_LIBS}
#            ${CURLCPP_SOURCE_DIR}/lib
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