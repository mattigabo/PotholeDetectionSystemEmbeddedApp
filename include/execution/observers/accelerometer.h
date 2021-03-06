//
// Created by Matteo Gabellini on 2018-12-21.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_ACCELEROMETER_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_ACCELEROMETER_H

#include "rxcpp/rx.hpp"

#include <vector>

#include <networking.h>

#include <opencv2/core.hpp>
#include <phdetection/io.hpp>

#include <accelerometer/accelerometer.h>
#include <accelerometer/features.h>
#include <gps/GPSDataStore.h>
#include <raspberrypi/led.h>

namespace observers {

    namespace accelerometer {

        typedef std::pair<phd::devices::gps::Coordinates, std::vector<phd::devices::accelerometer::Acceleration>> GPSWithAccelerations;
        typedef std::pair<phd::devices::gps::Coordinates, cv::Mat> GPSWithMat;
        typedef std::pair<phd::devices::gps::Coordinates, phd::devices::accelerometer::data::Features> GPSWithFeatures;
        typedef phd::configurations::MLOptions<phd::configurations::SVMParams> SVMAxelConfig;

        rxcpp::composite_subscription runAccelerometerObserver(phd::devices::gps::GPSDataStore *gpsDataStore,
                                                               phd::devices::accelerometer::Accelerometer *accelerometer,
                                                               phd::devices::accelerometer::data::Axis &observationAxis,
                                                               phd::io::Configuration &phdConfig,
                                                               SVMAxelConfig &svmAxelConfig,
                                                               phd::configurations::ServerConfig &serverConfig,
                                                               phd::devices::raspberry::led::Led *dataTransferringNotificationLed);

        rxcpp::composite_subscription runAccelerometerValuesWriter(phd::devices::gps::GPSDataStore *gpsDataStore,
                                                                   phd::devices::accelerometer::Accelerometer *accelerometer,
                                                                   std::string axelOutputLocation);
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_ACCELEROMETER_H