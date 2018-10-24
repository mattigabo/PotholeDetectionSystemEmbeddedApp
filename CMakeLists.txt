cmake_minimum_required(VERSION 3.1)
project(PotholeDetectionSystemEmbeddedApp VERSION 0.0.2)

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
    set(MiNMEA_DIR "${PROJECT_SOURCE_DIR}/lib/minmea")

    find_package(OpenCV REQUIRED)
    #    find_package(RapidJSON REQUIRED)
    find_package(PotholeDetection REQUIRED)
    find_package(Threads)

    if(${APPLE})
        find_package(CURL REQUIRED)
    else()
        find_package(CURL REQUIRED PATHS "$ENV{CURL_DIR}/CMAKE")
    endif()



    add_library(libserialport STATIC IMPORTED)
    set_property(TARGET libserialport PROPERTY IMPORTED_LOCATION /usr/local/lib/libserialport.a)



    include_directories(
            "${OpenCV_INCLUDE_DIRS}"
            "${RapidJSON_DIR}/include"
            "${MiNMEA_DIR}"
            "${PotholeDetection_INCLUDE_DIRS}"
            "${CURL_INCLUDE_DIRS}"
            "${PROJECT_SOURCE_DIR}/include"
    )

    add_executable(
            PotholeDetectionSystemEmbeddedApp main.cpp
            src/camera.cpp include/camera.h
            src/networking.cpp include/networking.h
            src/serialport/SerialPort.cpp include/serialport/SerialPort.h
            src/serialport/SerialPortUtils.cpp include/serialport/SerialPortUtils.h
            src/serialport/SigrokSerialPortWrapper.cpp include/serialport/SigrokSerialPortWrapper.h
            src/gps/GPSDataStore.cpp include/gps/GPSDataStore.h
            src/gps/GPSDataUpdater.cpp include/gps/GPSDataUpdater.h
            #lib/minmea/minmea.c
            include/ConfigurationUtils.h src/ConfigurationUtils.cpp)

    # linking
    target_link_libraries(
            PotholeDetectionSystemEmbeddedApp
            ${PotholeDetection_LIBS}
            ${OpenCV_LIBS}
            ${CURL_LIBRARIES}
            pthread
    )

    if(${APPLE})
        find_library(IOKIT IOKit)
        find_library(CORE_FOUNDATION CoreFoundation)
        target_link_libraries(PotholeDetectionSystemEmbeddedApp libserialport ${IOKIT} ${CORE_FOUNDATION})
    else()
        target_link_libraries(PotholeDetectionSystemEmbeddedApp libserialport)
    endif()

    install(
            TARGETS PotholeDetectionSystemEmbeddedApp
            ARCHIVE         DESTINATION         "lib/static"
            LIBRARY         DESTINATION         "lib/shared"
            RUNTIME         DESTINATION         "bin"
            PUBLIC_HEADER   DESTINATION         "include/phdo"
    )

    return(1)
endif ()