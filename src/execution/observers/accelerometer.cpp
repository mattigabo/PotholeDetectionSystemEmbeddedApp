//
// Created by Matteo Gabellini on 2018-12-21.
//
#include "execution/observers/accelerometer.h"

#include <future>

#include <chrono>
#include <string>
#include <thread>
#include <time.h>

#include <execution/observables/accelerometer.h>
#include <fingerprint.h>
#include <execution/utils.h>

using namespace phd;

namespace observers {
    namespace accelerometer {

        void runAccelerometerObserver(phd::devices::gps::GPSDataStore *gpsDataStore,
                                      phd::devices::accelerometer::Accelerometer *accelerometer,
                                      phd::devices::accelerometer::data::Axis &observationAxis,
                                      phd::io::Configuration &phdConfig,
                                      SVMAxelConfig &svmAxelConfig,
                                      phd::configurations::ServerConfig &serverConfig,
                                      phd::devices::raspberry::led::Led *dataTransferingNotificationLed) {

            auto accelerometer_obs = observables::accelerometer::createAccelerometerValuesStream(accelerometer,
                    OBSERVATION_PERIOD_AT_50Hz);

            auto buffered_accelerations_with_gps = accelerometer_obs.buffer(30)
                    .map([gpsDataStore](std::vector<devices::accelerometer::Acceleration> v){
                        return std::make_pair(gpsDataStore->fetch(), v);
                    });

            buffered_accelerations_with_gps
            .map([observationAxis](GPSWithAccelerations gpsWithAccelerations) {
                return std::make_pair(
                        gpsWithAccelerations.first,
                        devices::accelerometer::data::toFeatures(
                                gpsWithAccelerations.second,
                                observationAxis
                        )
                );
            }).map([](GPSWithFeatures gpsWithFeatures) {

                auto v = std::vector<devices::accelerometer::data::Features>();

                v.emplace_back(gpsWithFeatures.second);

                return std::make_pair(
                        gpsWithFeatures.first,
                        devices::accelerometer::data::toMat(v)
                );
            }).map([svmAxelConfig](GPSWithMat gpsWithMat) {

                return std::make_pair(
                        gpsWithMat.first,
                        devices::accelerometer::data::normalize(
                                gpsWithMat.second,
                                svmAxelConfig.norm_range.first,
                                svmAxelConfig.norm_range.second,
                                svmAxelConfig.norm_method
                        )
                );

            }).map([svmAxelConfig](GPSWithMat gpsWithNormMat) {

                auto labels = devices::accelerometer::data::classify(gpsWithNormMat.second, svmAxelConfig.model);

                return std::make_pair(gpsWithNormMat.first, labels);

            }).filter([](GPSWithMat gpsWithLabels) {

                auto labels = gpsWithLabels.second.row(0);

                std::vector<int> l(labels.ptr<int>(0), labels.ptr<int>(0) + labels.cols);

                auto is_ph_label_present = std::find(l.begin(), l.end(), 1) != l.end();

                if (is_ph_label_present) {
                    std::cout << "PH found from Accelerometer @ [" <<
                              gpsWithLabels.first.longitude << "," << gpsWithLabels.first.latitude
                              << "]" << std::endl;
                } else {
                    std::cout << "NO PH found from Accelerometer @ [" <<
                              gpsWithLabels.first.longitude << "," << gpsWithLabels.first.latitude
                              << "]" << std::endl;
                }

                return is_ph_label_present;

            }).map([] (GPSWithMat gpsWithLabels){
                return gpsWithLabels.first;
            }).subscribe([serverConfig, dataTransferingNotificationLed](phd::devices::gps::Coordinates coordinates) {
                std::string position = toJSON(coordinates, fingerprint::getUID());

                auto f = std::async(std::launch::async, [position, serverConfig, dataTransferingNotificationLed]() {
                    dataTransferingNotificationLed->switchOn();
                    sendDataToServer(position, serverConfig);
                    dataTransferingNotificationLed->switchOff();
                });
            });

//            accelerometer_obs.as_blocking().subscribe();

        }
    }
}


