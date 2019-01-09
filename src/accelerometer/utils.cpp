//
// Created by Xander on 26/11/2018.
//

#include "accelerometer/utils.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

namespace phd {
    namespace devices {
        namespace accelerometer {
            namespace utils {

                std::vector<Acceleration> exctractAccelerationsFromDataSet(rapidjson::Document &config){

                    std::vector<Acceleration> accelerations = std::vector<Acceleration>();

                    assert(config.HasMember("rot_acc_y"));
                    assert(config.HasMember("rot_acc_x"));
                    assert(config.HasMember("rot_acc_z"));

                    assert(config["rot_acc_y"].IsArray());
                    assert(config["rot_acc_x"].IsArray());
                    assert(config["rot_acc_z"].IsArray());

                    assert(config["rot_acc_y"].GetArray().Capacity() == config["rot_acc_x"].GetArray().Capacity() &&
                           config["rot_acc_y"].GetArray().Capacity() == config["rot_acc_z"].GetArray().Capacity());

                    for (int i = 0; i < config["rot_acc_y"].GetArray().Capacity(); ++i) {
                        const auto &v_y = config["rot_acc_y"].GetArray()[i];
                        const auto &v_x = config["rot_acc_x"].GetArray()[i];
                        const auto &v_z = config["rot_acc_z"].GetArray()[i];
                        assert(v_y.IsFloat());
                        assert(v_x.IsFloat());
                        assert(v_z.IsFloat());
                        accelerations.push_back({ v_x.GetFloat() , v_y.GetFloat(), v_z.GetFloat()});
                    }

                    return accelerations;
                }

                std::vector<Acceleration> readAccelerationsInGFromFile(const std::string &file){
                    std::vector<Acceleration> accelerationsInG = std::vector<Acceleration>();

                    std::ifstream json(file, std::fstream::in);

                    rapidjson::IStreamWrapper wrapper(json);

                    rapidjson::Document config;
                    config.ParseStream(wrapper);

                    if (json.is_open() && config.IsObject()) {
                        std::vector<Acceleration> accelerationsInMSSquared = exctractAccelerationsFromDataSet(config);
                        for(const auto acceleration : accelerationsInMSSquared){
                            accelerationsInG.push_back(convertToG(acceleration));
                        }
                    } else {
                        std::cerr << "Error" << std::endl;

                        exit(-3);
                    }

                    json.close();

                    return accelerationsInG;
                }

                std::vector<Acceleration> readAccelerationsInGFromDataSet(const std::string &dataset){
                    std::cout << "Loading Acceleration from dataset..." << std::endl;
                    std::vector<Acceleration> accelerationsLoaded;
                    if (phd::io::is_dir(dataset.data())) {
                        std::vector<cv::String> globs;
                        cv::glob(dataset + "/*.json", globs);

                        for (const std::string ds : globs) {
                            const auto accelerations = phd::devices::accelerometer::utils::readAccelerationsInGFromFile(
                                    ds);
                            for(const auto acc : accelerations) {
                                accelerationsLoaded.push_back(acc);
                            }
                        }
                    } else if (phd::io::is_file(dataset.data())) {
                        const auto accelerations = phd::devices::accelerometer::utils::readAccelerationsInGFromFile(
                                dataset);
                        for(const auto acc : accelerations) {
                            accelerationsLoaded.push_back(acc);
                        }
                    } else {
                        std::cerr << "Undefined directory or file " << dataset << std::endl;
                        exit(-3);
                    }
                    std::cout << "Dataset Loading finished!" << std::endl;
                    return accelerationsLoaded;
                }

                DataSet readJSONDataset(const std::string &dataset) {

                    DataSet dataLoaded = {
                            std::vector<Anomaly>(),
                            std::vector<Acceleration>(),
                    };

                    std::ifstream json(dataset, std::fstream::in);

                    rapidjson::IStreamWrapper wrapper(json);

                    rapidjson::Document config;
                    config.ParseStream(wrapper);

                    if (json.is_open() && config.IsObject()) {

                        dataLoaded.accelerations = exctractAccelerationsFromDataSet(config);

                        assert(config.HasMember("anomalies"));
                        assert(config["anomalies"].IsArray());
                        for (const auto &a : config["anomalies"].GetArray()) {

                            assert(a.IsObject());
                            assert(a.HasMember("start"));
                            assert(a["start"].IsInt());
                            assert(a.HasMember("end"));
                            assert(a["end"].IsInt());
                            assert(a.HasMember("type"));
                            assert(a["type"].IsString());

                            dataLoaded.anomalies.push_back(Anomaly {
                                .starts = a["start"].GetInt(),
                                .ends = a["end"].GetInt(),
                                .type = a["type"].GetString()
                            });
                        }

                    } else {
                        std::cerr << "Error" << std::endl;

                        exit(-3);
                    }

                    json.close();

                    return dataLoaded;
                }

                void printAccelerationValues(Acceleration acceleration, std::string measureUnit){
                    std::cout << "Accelerometer: [ " <<
                        acceleration.X << " " << measureUnit << " on X,  " <<
                        acceleration.Y << " " << measureUnit << " on Y,  " <<
                        acceleration.Z << " " << measureUnit << " on Z ]"  << std::endl;
                }

                Acceleration convertToMSSquared(Acceleration accelerationInG){
                    phd::devices::accelerometer::Acceleration accInMeterSecondSquared = {
                            accelerationInG.X * devices::accelerometer::data::g,
                            accelerationInG.Y * devices::accelerometer::data::g,
                            accelerationInG.Z * devices::accelerometer::data::g
                    };
                    return accInMeterSecondSquared;
                }

                Acceleration convertToG(Acceleration accelerationInMSSquared){
                    phd::devices::accelerometer::Acceleration accInMeterSecondSquared = {
                            accelerationInMSSquared.X / devices::accelerometer::data::g,
                            accelerationInMSSquared.Y / devices::accelerometer::data::g,
                            accelerationInMSSquared.Z / devices::accelerometer::data::g
                    };
                    return accInMeterSecondSquared;
                }

                bool toFeatures(const DataSet &dataset,
                                const std::string &axis,
                                const int window_size,
                                std::function<int(int)> sliding_logic,
                                std::vector<phd::devices::accelerometer::data::Features> &features,
                                std::vector<int> &labels) {

                    std::vector<float> v = std::vector<float>();

                    if(axis == "x" || axis == "X") {
                        for (const auto &a : dataset.accelerations){
                            v.push_back(a.X);
                        }
                    } else if(axis == "y" || axis == "Y"){
                        for (const auto &a : dataset.accelerations){
                            v.push_back(a.Y);
                        }
                    } else {
                        for (const auto &a : dataset.accelerations){
                            v.push_back(a.Z);
                        }
                    }

                    int slider = 0;

                    while (slider < v.size() - window_size) {

                        auto res = phd::devices::accelerometer::data::getWindow(v, window_size, slider);

                        auto a_copy = std::vector<Anomaly>();

                        // Gets only those anomalies that contains the starting index of the slider and
                        // Only for those windows that contains at least the 100% of the labeled malformation
                        std::copy_if(dataset.anomalies.begin(), dataset.anomalies.end(),
                                std::back_inserter(a_copy),
                                [&](const Anomaly& a) {
                            return slider >= a.starts && slider + window_size <= a.ends;
//                            && (float)(slider - a.starts) / (float) (a.ends - a.starts) < 0.5;
                        });

                        // If contains at least an anomaly then
                        // add the label 1 for the actual features set
                        // else set the label 0
                        labels.push_back((!a_copy.empty()) ? 1 : 0);

                        auto ft = phd::devices::accelerometer::data::getFeatures(res);

                        features.push_back(ft);

                        slider += sliding_logic(window_size);
                    }

                    return true;
                }
            }
        }
    }
}
