//
// Created by Xander on 26/11/2018.
//

#include "accelerometerutils.h"

#include <iostream>
#include <fstream>
#include <functional>
#include <algorithm>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

namespace phd::devices::accelerometer::utils {

    RawData readJSONDataset(const std::string &dataset) {

        RawData rawData = {
                std::vector<Anomaly>(),
                std::vector<float>(),
                std::vector<float>(),
                std::vector<float>()
        };

        std::ifstream json(dataset, std::fstream::in);

        rapidjson::IStreamWrapper wrapper(json);

        rapidjson::Document config;
        config.ParseStream(wrapper);

        if (json.is_open() && config.IsObject()) {

            assert(config.HasMember("rot_acc_y"));
            assert(config.HasMember("rot_acc_x"));
            assert(config.HasMember("rot_acc_z"));
            assert(config.HasMember("anomalies"));

            assert(config["rot_acc_y"].IsArray());
            assert(config["rot_acc_x"].IsArray());
            assert(config["rot_acc_z"].IsArray());
            assert(config["anomalies"].IsArray());

            assert(config["rot_acc_y"].GetArray().Capacity() == config["rot_acc_x"].GetArray().Capacity() &&
                           config["rot_acc_y"].GetArray().Capacity() == config["rot_acc_z"].GetArray().Capacity());

            for (int i = 0; i < config["rot_acc_y"].GetArray().Capacity(); ++i) {
                const auto &v_y = config["rot_acc_y"].GetArray()[i];
                const auto &v_x = config["rot_acc_x"].GetArray()[i];
                const auto &v_z = config["rot_acc_z"].GetArray()[i];
                assert(v_y.IsFloat());
                assert(v_x.IsFloat());
                assert(v_z.IsFloat());
                rawData.y.push_back(v_y.GetFloat());
                rawData.x.push_back(v_x.GetFloat());
                rawData.z.push_back(v_z.GetFloat());
            }

            for (const auto &a : config["anomalies"].GetArray()) {

                assert(a.IsObject());
                assert(a.HasMember("start"));
                assert(a["start"].IsInt());
                assert(a.HasMember("end"));
                assert(a["end"].IsInt());
                assert(a.HasMember("type"));
                assert(a["type"].IsString());

                rawData.anomalies.push_back(Anomaly {
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

        return rawData;
    }

    bool toFeatures(const RawData &raw, const std::string &axis, std::function<int(int)> sliding_logic,
            std::vector<phd::devices::accelerometer::Features> &features, std::vector<int> &labels){

        const auto v = (
                axis == "x" || axis == "X" ? raw.x :
                axis == "y" || axis == "Y" ? raw.y :
                raw.z
            );

        int
            slider = 0,
            window_size = phd::devices::accelerometer::std_coefficients.windows_size;

        while (slider < v.size() - window_size) {

            auto res = phd::devices::accelerometer::getWindow(v, window_size, slider);

            auto a_copy = std::vector<Anomaly>();

            std::copy_if(raw.anomalies.begin(), raw.anomalies.end(),
                    std::back_inserter(a_copy),
                    [&](const Anomaly& a) {
                        return slider > a.starts && slider < a.ends
                        // Only gets those features that cover more than the 50% on the labeled malformation
                        && (float)(slider - a.starts) / (float) (a.ends - a.starts) < 0.5;
                    });

            if (!a_copy.empty()) {
                labels.push_back(1);
            } else {
                labels.push_back(0);
            }

            auto ft = phd::devices::accelerometer::getFeatures(res);

            features.push_back(ft);

            slider += sliding_logic(window_size);
        }

        return true;
    }
}
