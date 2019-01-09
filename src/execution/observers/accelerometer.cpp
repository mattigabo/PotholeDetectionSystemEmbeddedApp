//
// Created by Matteo Gabellini on 2018-12-21.
//
#include "execution/observers/accelerometer.h"

#include <future>
#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include <thread>
#include <time.h>

#include <execution/observables/accelerometer.h>
#include <fingerprint.h>
#include <execution/utils.h>
#include <accelerometer/utils.h>
#include <execution/observers/accelerometer.h>


using namespace phd;

namespace observers {
    namespace accelerometer {

        rxcpp::composite_subscription
            runAccelerometerObserver(phd::devices::gps::GPSDataStore *gpsDataStore,
                                     phd::devices::accelerometer::Accelerometer *accelerometer,
                                     phd::devices::accelerometer::data::Axis &observationAxis,
                                     phd::io::Configuration &phdConfig,
                                     SVMAxelConfig &svmAxelConfig,
                                     phd::configurations::ServerConfig &serverConfig,
                                     phd::devices::raspberry::led::Led *dataTransferringNotificationLed) {

            auto subscription = rxcpp::composite_subscription();

            auto accelerometer_obs = observables::accelerometer::createAccelerometerObservable(accelerometer,
                                                                                               observables::accelerometer::REFRESH_PERIOD_AT_50Hz);

            auto buffered_accelerations_with_gps = accelerometer_obs.map([](phd::devices::accelerometer::Acceleration accelerationInG){
                        return phd::devices::accelerometer::utils::convertToMSSquared(accelerationInG);
                    }).buffer(svmAxelConfig.window, -svmAxelConfig.slider)
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

            })
//            .map([svmAxelConfig](GPSWithMat gpsWithMat) {
//
////                std::cout << "Pre Norm: " << gpsWithMat.second << std::endl;
//
//                gpsWithMat.second.push_back(svmAxelConfig.min);
//                gpsWithMat.second.push_back(svmAxelConfig.max);
//
//                auto normalizedMat = std::make_pair(
//                    gpsWithMat.first,
//                    devices::accelerometer::data::normalize(
//                            gpsWithMat.second,
//                            svmAxelConfig.norm_range.first,
//                            svmAxelConfig.norm_range.second,
//                            svmAxelConfig.norm_method
//                    ).row(0)
//                );
//
////                std::cout << "Post Norm: " << normalizedMat.second << std::endl;
//
//                return normalizedMat;
//
//            })
            .map([svmAxelConfig](GPSWithMat gpsWithNormMat) {

                auto labels = devices::accelerometer::data::classify(gpsWithNormMat.second, svmAxelConfig.model);

                return std::make_pair(gpsWithNormMat.first, labels);

            }).filter([](GPSWithMat gpsWithLabels) {

                auto labels = gpsWithLabels.second.at<int>(0,0);

                auto is_ph_label_present = labels == 1;

//                std::cout << "Label: " << labels << std::endl;

                if (is_ph_label_present) {
                    std::cout << "OK: PH found from Accelerometer @ [" <<
                              gpsWithLabels.first.longitude << "," << gpsWithLabels.first.latitude
                              << "]" << std::endl;
                } else {
                    std::cout << "NO: PH found from Accelerometer @ [" <<
                              gpsWithLabels.first.longitude << "," << gpsWithLabels.first.latitude
                              << "]" << std::endl;
                }

                return is_ph_label_present;

            }).map([] (GPSWithMat gpsWithLabels){
                return gpsWithLabels.first;
            }).subscribe(
                    subscription,
                    [subscription, serverConfig, dataTransferringNotificationLed](phd::devices::gps::Coordinates coordinates) {

                        std::string position = toJSON(coordinates, fingerprint::getUID());

                        auto f = std::async(std::launch::async, [position, serverConfig, dataTransferringNotificationLed]() {
                            dataTransferringNotificationLed->switchOn();
                            sendDataToServer(position, serverConfig);
                            dataTransferringNotificationLed->switchOff();
                        });

                    }, []() {
                        std::cout << "Axel Stream Classifier has COMPLETED." << std::endl;
                    }
            );

            return subscription;

        }

        rxcpp::composite_subscription
            runAccelerometerValuesWriter(phd::devices::gps::GPSDataStore *gpsDataStore,
                                         phd::devices::accelerometer::Accelerometer *accelerometer,
                                         std::string axelOutputLocation) {

            auto output_file = new std::ofstream(axelOutputLocation);
            auto accelerometer_obs = observables::accelerometer::createAccelerometerObservable(
                    accelerometer,
                    observables::accelerometer::REFRESH_PERIOD_AT_50Hz
            );

            auto subscription = rxcpp::composite_subscription();

            accelerometer_obs.subscribe(
                    subscription,
                    [subscription, output_file, gpsDataStore](phd::devices::accelerometer::Acceleration axel) {
                        if (output_file->is_open()) {
                            auto gps = gpsDataStore->fetch();
                            (*output_file)
                                    << axel.X << "," << axel.Y << "," << axel.Z << ","
                                    << gps.longitude << "," << gps.latitude
                                    << std::endl;
                        }
                    }, [output_file](){
                        std::cout << "Axel Stream Writer has COMPLETED." << std::endl;
                        output_file->close();
                    }
            );

            return subscription;
        }
    }
}


