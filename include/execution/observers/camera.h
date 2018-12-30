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

        typedef std::pair<phd::devices::gps::Coordinates, cv::Mat> GPSWithMat;
        typedef std::pair<phd::devices::gps::Coordinates, std::vector<phd::ontologies::Features>> GPSWithFeatures;

        void runCameraObserver(phd::devices::gps::GPSDataStore *gpsDataStore,
                               phd::io::Configuration &phdConfig,
                               phd::configurations::CVArgs &cvConfig,
                               phd::configurations::ServerConfig &serverConfig,
                               phd::devices::raspberry::led::Led *dataTransferingNotificationLed);
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CAMERA_H
