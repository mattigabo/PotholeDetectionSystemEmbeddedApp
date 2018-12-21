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

                typedef struct RawData {
                    std::vector<Anomaly> anomalies;
                    std::vector<float> x;
                    std::vector<float> y;
                    std::vector<float> z;
                } RawData;

                RawData readJSONDataset(const std::string &dataset);

                bool toFeatures(const RawData &raw, const std::string &axis, std::function<int(int)> sliding_logic,
                        std::vector<phd::devices::accelerometer::data::Features> &features, std::vector<int> &labels);

            }
        }
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELEROMETER_UTILS_H

