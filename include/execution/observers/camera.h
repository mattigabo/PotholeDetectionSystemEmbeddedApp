//
// Created by Xander on 04/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_CAMERA_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_CAMERA_H

#include "rxcpp/rx.hpp"

#include <vector>

#include <phdetection/ontologies.hpp>
#include <phdetection/core.hpp>

#include <networking.h>
#include <gps/GPSDataStore.h>

#include <raspberrypi/led.h>

namespace observers {
    namespace camera {

        typedef phd::devices::gps::Coordinates GPSCoordinates;

        typedef std::pair<GPSCoordinates, cv::Mat> GPSWithMat;
        typedef std::pair<GPSCoordinates, std::vector<phd::ontologies::Features>> GPSWithFeatures;

        typedef std::pair<GPSWithMat, cv::Mat> GPSWithMatAndCapture;
        typedef std::pair<GPSWithMat, std::vector<phd::ontologies::Features>> GPSWithFeaturesAndCapture;

        rxcpp::composite_subscription runCameraObserver(phd::devices::gps::GPSDataStore *gpsDataStore,
                                                        phd::io::Configuration &phdConfig,
                                                        phd::configurations::CVArgs &cvConfig,
                                                        phd::configurations::ServerConfig &serverConfig,
                                                        phd::devices::raspberry::led::Led *cameraIsShootingLed,
                                                        phd::devices::raspberry::led::Led *dataTransferringNotificationLed);

        rxcpp::composite_subscription runCameraObserverWithCaptureSaver(phd::devices::gps::GPSDataStore *gpsDataStore,
                                                        phd::io::Configuration &phdConfig,
                                                        phd::configurations::CVArgs &cvConfig,
                                                        phd::configurations::ServerConfig &serverConfig,
                                                        phd::devices::raspberry::led::Led *cameraIsShootingLed,
                                                        phd::devices::raspberry::led::Led *dataTransferringNotificationLed,
                                                        std::string &posCapturesSaveLocation);
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CAMERA_H
