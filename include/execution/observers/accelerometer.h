//
// Created by Matteo Gabellini on 2018-12-21.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_ACCELEROMETER_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_ACCELEROMETER_H

#include "rxcpp/rx.hpp"

#include <chrono>
#include <vector>
#include <string>
#include <thread>
#include <time.h>

#include <phdetection/ontologies.hpp>
#include <phdetection/core.hpp>

#include <networking.h>
#include <accelerometer/accelerometer.h>
#include <accelerometer/ml.h>
#include <gps/GPSDataStore.h>

namespace observers {
    namespace accelerometer {

        const long OBSERVATION_PERIOD_AT_50Hz = 20L; //50hz = 20ms

        typedef std::pair<phd::devices::gps::Coordinates, std::vector<float>> GPSWithAccelerations;
        typedef std::pair<phd::devices::gps::Coordinates, phd::devices::accelerometer::ml::Features> GPSWithFeatures;

        void runAccelerometerObserver(phd::devices::gps::GPSDataStore *gpsDataStore,
                                      phd::devices::accelerometer::Accelerometer *accelerometer,
                                      phd::io::Configuration &phdConfig,
                                      phd::configurations::CVArgs &cvConfig,
                                      phd::configurations::ServerConfig &serverConfig,
                                      phd::devices::accelerometer::ml::Axis observationAxis);
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_ACCELEROMETER_H
