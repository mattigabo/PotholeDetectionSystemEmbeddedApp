//
// Created by Xander on 04/12/2018.
//

#include "execution/observers/camera.h"
#include <execution/observables/camera.h>
#include <execution/observables/gps.h>
#include <execution/utils.h>

namespace observers {
    namespace camera {

        void runCameraObserver(phd::devices::gps::GPSDataStore *gpsDataStore,
                               phd::io::Configuration &phdConfig,
                               phd::configurations::CVArgs &cvConfig,
                               phd::configurations::ServerConfig &serverConfig) {

            auto gps_obs = observables::gps::createGPSObservable(gpsDataStore, 1500L);
            auto camera_obs = observables::camera::createCameraObservable(gps_obs);

            camera_obs.map([&](GPSWithMat image) {

                if (cvConfig.rotate) {
                    cv::rotate(image.second, image.second, cv::ROTATE_180);
                }

                return image;

            }).map([&](GPSWithMat gpsWithCapture){

                std::vector<phd::ontologies::Features> features =
                        phd::getFeatures(gpsWithCapture.second, phdConfig);

                return GPSWithFeatures(gpsWithCapture.first, features);

            }).map([&](GPSWithFeatures gpsWithFeatures){
                cv::Mat labels;

                try {
                    labels = phd::classify(cvConfig.method, cvConfig.svm, cvConfig.bayes, gpsWithFeatures.second);
                } catch(phd::UndefinedMethod &ex)  {
                    cerr << "ERROR: " << ex.what() << endl;
                    exit(-1);
                }

                return GPSWithMat(gpsWithFeatures.first, labels);

            }).filter([&](GPSWithMat gpsWithLabels){

                return gpsWithLabels.second.rows != 0;

            }).subscribe([&](GPSWithMat gpsWithLabels){
                auto labels = gpsWithLabels.second.row(0);

                vector<int> l(labels.ptr<int>(0), labels.ptr<int>(0) + labels.cols);

                if (std::find(l.begin(), l.end(), 1) != l.end() ||
                    std::find(l.begin(), l.end(), 2) != l.end()) {

                    std::string position = toJSON(gpsWithLabels.first);

                    sendDataToServer(position, serverConfig);
                }
            });
        }

    }
}