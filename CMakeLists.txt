cmake_minimum_required(VERSION 3.10)
project(PotholeDetectionObserver VERSION 0.0.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Where to find CMake modules and OpenCV
if ("$ENV{OpenCV_DIR}" STREQUAL "")
    message("ERROR: The OPENCV environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{PotholeDetection_DIR}" STREQUAL "")
    message("ERROR: The PhD environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{CURL_DIR}" STREQUAL "")
    message("ERROR: The cURL environmental/global variable is not set or is empty")
    return(0)
else ()
    message("$ENV{OpenCV_DIR}")
    message("$ENV{PotholeDetection_DIR}")
    message("$ENV{CURL_DIR}")

#    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules")
    set(RapidJSON_DIR "${PROJECT_SOURCE_DIR}/lib/RapidJSON")

    find_package(OpenCV REQUIRED)
#    find_package(RapidJSON REQUIRED)
    find_package(PotholeDetection REQUIRED)

    find_package(CURL REQUIRED PATHS "$ENV{CURL_DIR}/CMAKE")


    include_directories(
            "${OpenCV_INCLUDE_DIRS}"
            "${RapidJSON_DIR}/include"
            "${PotholeDetection_INCLUDE_DIRS}"
            "${CURL_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}/include"
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
            ${PotholeDetection_LIBS}
            ${OpenCV_LIBS}
            ${CURL_LIBRARIES}
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