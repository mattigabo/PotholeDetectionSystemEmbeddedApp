//
// Created by Matteo Gabellini on 2018-12-21.
//
#include "execution/observers/accelerometer.h"

#include <chrono>
#include <string>
#include <thread>
#include <time.h>

#include <execution/observables/accelerometer.h>
#include <fingerprint.h>

namespace observers {
    namespace accelerometer {

        void runAccelerometerObserver(phd::devices::gps::GPSDataStore *gpsDataStore,
                                      phd::devices::accelerometer::Accelerometer *accelerometer,
                                      phd::io::Configuration &phdConfig,
                                      phd::configurations::CVArgs &cvConfig,
                                      phd::configurations::ServerConfig &serverConfig,
                                      phd::devices::accelerometer::data::Axis &observationAxis) {

            auto accelerometer_obs = observables::accelerometer::createAccelerometerValuesStream(accelerometer,
                    OBSERVATION_PERIOD_AT_50Hz);

            auto buffered_accelerations_with_gps = accelerometer_obs.buffer(30)
                    .map([gpsDataStore](std::vector<phd::devices::accelerometer::Acceleration> v){
                        return std::make_pair(gpsDataStore->fetch(), v);
                    }).publish();

            buffered_accelerations_with_gps.connect();

        }
    }
}


