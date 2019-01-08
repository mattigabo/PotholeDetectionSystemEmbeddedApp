//
// Created by Xander on 26/11/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELEROMETER_UTILS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELEROMETER_UTILS_H

#include <vector>
#include <string>
#include <functional>
#include "accelerometer/features.h"

namespace phd {
    namespace devices{
        namespace accelerometer {
            namespace utils {

                typedef struct Anomaly {
                    int starts;
                    int ends;
                    std::string type;
                } Anomaly;

                typedef struct DataSet {
                    std::vector<Anomaly> anomalies;
                    std::vector<Acceleration> accelerations;
                } DataSet;

                DataSet readJSONDataset(const std::string &dataset);

                std::vector<Acceleration> readAccelerationsInGFromDataSet(const std::string &dataset);

                void printAccelerationValues(Acceleration acceleration, std::string measureUnit);

                Acceleration convertToMSSquared(Acceleration accelerationInG);

                Acceleration convertToG(Acceleration accelerationInMSSquared);

                bool toFeatures(const DataSet &dataset, const std::string &axis, std::function<int(int)> sliding_logic,
                        std::vector<phd::devices::accelerometer::data::Features> &features, std::vector<int> &labels);

            }
        }
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELEROMETER_UTILS_H

