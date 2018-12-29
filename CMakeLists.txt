cmake_minimum_required(VERSION 3.1)
project(PotholeDetectionSystem_EmbeddedApp VERSION 0.0.2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# Where to find CMake modules and OpenCV
if ("$ENV{OpenCV_DIR}" STREQUAL "")
    message("ERROR: The OPENCV installation environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{PotholeDetection_DIR}" STREQUAL "")
    message("ERROR: The PhD installation environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{CURL_DIR}" STREQUAL "")
    message("ERROR: The cURL installation environmental/global variable is not set or is empty")
    return(0)
elseif("$ENV{MiNMEA_DIR}" STREQUAL "")
    message("ERROR: The minmea installation environmental/global variable is not set or is empty")
    return(0)
elseif("${WIN32}" AND "$ENV{LibSerialPort_DIR}" STREQUAL "")
    message("ERROR: The libserialport installation environmental/global variable is not set or is empty")
    return(0)
else ()

    message("$ENV{OpenCV_DIR}")
    message("$ENV{PotholeDetection_DIR}")
    message("$ENV{CURL_DIR}")
    message("$ENV{MiNMEA_DIR}")
    message("$ENV{LibSerialPort_DIR}")

    message("Init git submodules...")
    execute_process(COMMAND "git submodule init")
    execute_process(COMMAND "git submodule update --recursive")

    #    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake-modules")
    set(RapidJSON_DIR "${PROJECT_SOURCE_DIR}/lib/RapidJSON")
    set(RxC++_DIR "${PROJECT_SOURCE_DIR}/lib/RxCpp")
    set(NunchuckAdapter_DIR "${PROJECT_SOURCE_DIR}/lib/nunchuck-adapter")

    find_package(OpenCV REQUIRED)
    find_package(PotholeDetection REQUIRED)
    find_package(MiNMEA REQUIRED)
    find_package(Threads)

    if(${APPLE})
        find_package(CURL REQUIRED)
        add_library(libserialport STATIC IMPORTED)
        set_property(TARGET libserialport PROPERTY IMPORTED_LOCATION /usr/local/lib/libserialport.a)
        set(LibSerialPort_INCLUDE /usr/local/include/)
    else()
        find_package(CURL REQUIRED PATHS "$ENV{CURL_DIR}/CMAKE" "$ENV{CURL_DIR}/CMake" "$ENV{CURL_DIR}/lib/cmake/CURL")

        if ("${CURL_INCLUDE_DIRS}" STREQUAL "" OR "${CURL_LIBRARIES}" STREQUAL "")
            set(CURL_INCLUDE_DIRS "$ENV{CURL_DIR}/include/")
            set(CURL_BINARIES "$ENV{CURL_DIR}/bin/")
            file(GLOB CURL_LIBRARIES "$ENV{CURL_DIR}/lib/lib*")
        endif()

        message("CURL found.")
        message("Headers: "${CURL_INCLUDE_DIRS})
        message("Libraries: "${CURL_LIBRARIES})
        message("Binaries: "${CURL_BINARIES})

        if (${WIN32})
            add_library(libserialport STATIC IMPORTED)
            set_property(TARGET libserialport PROPERTY IMPORTED_LOCATION "$ENV{LibSerialPort_DIR}/lib/libserialport.dll.a")
#            file(GLOB LibSerialPort_INCLUDE "$ENV{LibSerialPort_DIR}/include/*.h")
            set(LibSerialPort_INCLUDE "$ENV{LibSerialPort_DIR}/include")
        else() # Linux
            add_library(libserialport STATIC IMPORTED)
            set_property(TARGET libserialport PROPERTY IMPORTED_LOCATION /usr/local/lib/libserialport.a)
            set(LibSerialPort_INCLUDE /usr/local/include/)
        endif()
    endif()

#    message(OCV:"${OpenCV_INCLUDE_DIRS}")
#    message(RJ:"${RapidJSON_DIR}/include")
#    message(MinMEA:"${MiNMEA_INCLUDE_DIRS}")
#    message(PHD:"${PotholeDetection_INCLUDE_DIRS}")
#    message(CURL:"${CURL_INCLUDE_DIRS}")
#    message(LSP:"${LibSerialPort_INCLUDE}")
#    message(PROJECT:"${PROJECT_SOURCE_DIR}/include")

    include_directories(
            "${OpenCV_INCLUDE_DIRS}"
            "${RapidJSON_DIR}/include"
            "${RxC++_DIR}/Rx/v2/src"
            "${MiNMEA_INCLUDE_DIRS}"
            "${PotholeDetection_INCLUDE_DIRS}"
            "${CURL_INCLUDE_DIRS}"
            "${LibSerialPort_INCLUDE}"
            "${PROJECT_SOURCE_DIR}/include"
            "${NunchuckAdapter_DIR}/include"
    )

    add_executable(
            PotholeDetectionSystemEmbeddedApp main.cpp

            include/camera.h                                src/camera.cpp
            include/networking.h                            src/networking.cpp

            include/serialport/SerialPort.h                 src/serialport/SerialPort.cpp
            include/serialport/SerialPortUtils.h            src/serialport/SerialPortUtils.cpp
            include/serialport/SigrokSerialPortWrapper.h    src/serialport/SigrokSerialPortWrapper.cpp

            include/gps/GPSDataStore.h                      src/gps/GPSDataStore.cpp
            include/gps/GPSDataUpdater.h                    src/gps/GPSDataUpdater.cpp
            include/configurationutils.h                    src/configurationutils.cpp
            include/executionmodes.h                        src/executionmodes.cpp

            include/raspberrypi/led.h src/raspberrypi/led.cpp
            include/raspberrypi/raspberrypiutils.h          src/raspberrypi/raspberrypiutils.cpp

            include/accelerometer/features.h                src/accelerometer/features.cpp
            include/accelerometer/utils.h                   src/accelerometer/utils.cpp
            include/accelerometer/accelerometer.h           src/accelerometer/accelerometer.cpp

            include/fingerprint.h                           src/fingerprint.cpp

            include/execution/modes.h                       src/execution/modes.cpp
            include/execution/utils.h                       src/execution/utils.cpp
            include/execution/test.h                        src/execution/test.cpp

            include/execution/observers/camera.h            src/execution/observers/camera.cpp
            include/execution/observers/accelerometer.h     src/execution/observers/accelerometer.cpp

            include/execution/observables/camera.h          src/execution/observables/camera.cpp
            include/execution/observables/gps.h             src/execution/observables/gps.cpp
            include/execution/observables/accelerometer.h   src/execution/observables/accelerometer.cpp
            include/execution/observers/gps.h src/execution/observers/gps.cpp)

    # linking
    target_link_libraries(
            PotholeDetectionSystemEmbeddedApp
            ${PotholeDetection_LIBS}
            ${OpenCV_LIBS}
            ${CURL_LIBRARIES}
            ${MiNMEA_LIBS}
            pthread
    )

    if(${APPLE})
        find_library(IOKIT IOKit)
        find_library(CORE_FOUNDATION CoreFoundation)
        target_link_libraries(PotholeDetectionSystemEmbeddedApp libserialport ${IOKIT} ${CORE_FOUNDATION})
    else()
        target_link_libraries(PotholeDetectionSystemEmbeddedApp libserialport)
    endif()

    #check if is arm architecture
    EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
    message(STATUS "Architecture: ${ARCHITECTURE}" )
    if("${ARCHITECTURE}" STREQUAL "armv7l" )
        find_library(wiringPi_LIB wiringPi)
        target_link_libraries(PotholeDetectionSystemEmbeddedApp ${wiringPi_LIB})
    endif()

    install(
            TARGETS PotholeDetectionSystemEmbeddedApp
            ARCHIVE         DESTINATION         "lib/static"
            LIBRARY         DESTINATION         "lib/shared"
            RUNTIME         DESTINATION         "bin"
            PUBLIC_HEADER   DESTINATION         "include"
    )

    return(1)
endif ()