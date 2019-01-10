//
// Created by Xander on 04/12/2018.
//

#include "execution/observers/camera.h"
#include <execution/observables/camera.h>
#include <execution/observables/gps.h>
#include <execution/utils.h>
#include <fingerprint.h>
#include <execution/observers/camera.h>


using namespace std;

namespace observers {
    namespace camera {

        rxcpp::composite_subscription runCameraObserver(
                phd::devices::gps::GPSDataStore *gpsDataStore,
                phd::io::Configuration &phdConfig,
                phd::configurations::CVArgs &cvConfig,
                phd::configurations::ServerConfig &serverConfig,
                phd::devices::raspberry::led::Led *dataTransferringNotificationLed) {

            auto subscription = rxcpp::composite_subscription();

            auto gps_obs = observables::gps::createGPSObservable(gpsDataStore, 1500L);

            auto camera_obs = observables::camera::createCameraObservable(gps_obs);

            camera_obs.map([cvConfig](GPSWithMat image) {

                if (cvConfig.rotate) {
                    cv::rotate(image.second, image.second, cv::ROTATE_180);
                }

                return image;

            }).map([phdConfig](GPSWithMat gpsWithCapture){

                std::vector<phd::ontologies::Features> features =
                        phd::getFeatures(gpsWithCapture.second, phdConfig);

                return GPSWithFeatures(gpsWithCapture.first, features);

            }).map([cvConfig](GPSWithFeatures gpsWithFeatures){

                cv::Mat labels;

                try {
                    labels = phd::classify(cvConfig.method, cvConfig.svm, cvConfig.bayes, gpsWithFeatures.second);
                } catch(phd::UndefinedMethod &ex)  {
                    cerr << "ERROR: " << ex.what() << endl;
                    exit(-1);
                }

                return GPSWithMat(gpsWithFeatures.first, labels);

            }).filter([](GPSWithMat gpsWithLabels){

                return gpsWithLabels.second.rows != 0;

            }).filter([](GPSWithMat gpsWithLabels){

                auto labels = gpsWithLabels.second.row(0);

                vector<int> l(labels.ptr<int>(0), labels.ptr<int>(0) + labels.cols);

                auto is_ph_label_present = std::find(l.begin(), l.end(), 1) != l.end() ||
                                           std::find(l.begin(), l.end(), 2) != l.end();

                if (is_ph_label_present) {
                    std::cout << "PH found from Camera @ [" <<
                            gpsWithLabels.first.longitude << "," << gpsWithLabels.first.latitude
                            << "]" << std::endl;
                } else {
                    std::cout << "NO PH found from camera @ [" <<
                              gpsWithLabels.first.longitude << "," << gpsWithLabels.first.latitude
                              << "]" << std::endl;
                }

                return is_ph_label_present;

            }).map([](GPSWithMat gpsWithLabels) {
                return gpsWithLabels.first;
            }).subscribe(
                    subscription,
                    [subscription, serverConfig, dataTransferringNotificationLed](phd::devices::gps::Coordinates coordinates){
                        std::string position = toJSON(coordinates, fingerprint::getUID());

                        auto f = std::async(std::launch::async, [position, serverConfig, dataTransferringNotificationLed]() {
                            dataTransferringNotificationLed->switchOn();
                            sendDataToServer(position, serverConfig);
                            dataTransferringNotificationLed->switchOff();
                        });

                    }, []() {
                        std::cout << "Camera Image Classifier has COMPLETED." << std::endl;
                    }
            );

            return subscription;
        }

        rxcpp::composite_subscription runCameraObserverWithCaptureSaver(phd::devices::gps::GPSDataStore *gpsDataStore,
                                                                        phd::io::Configuration &phdConfig,
                                                                        phd::configurations::CVArgs &cvConfig,
                                                                        phd::configurations::ServerConfig &serverConfig,
                                                                        phd::devices::raspberry::led::Led *dataTransferringNotificationLed,
                                                                        std::string &posCapturesSaveLocation) {

            auto subscription = rxcpp::composite_subscription();

            int counter = 1;

            auto gps_obs = observables::gps::createGPSObservable(gpsDataStore, 1500L);

            auto camera_obs = observables::camera::createCameraObservable(gps_obs);

            camera_obs.map([cvConfig](GPSWithMat image) {

                if (cvConfig.rotate) {
                    cv::rotate(image.second, image.second, cv::ROTATE_180);
                }

                return image;

            }).map([phdConfig](GPSWithMat gpsWithCapture){

                std::vector<phd::ontologies::Features> features =
                        phd::getFeatures(gpsWithCapture.second, phdConfig);

                return GPSWithFeaturesAndCapture(gpsWithCapture, features);

            }).map([cvConfig](GPSWithFeaturesAndCapture gpsWithFeatures){

                cv::Mat labels;

                try {
                    labels = phd::classify(cvConfig.method, cvConfig.svm, cvConfig.bayes, gpsWithFeatures.second);
                } catch(phd::UndefinedMethod &ex)  {
                    cerr << "ERROR: " << ex.what() << endl;
                    exit(-1);
                }

                return GPSWithMatAndCapture(gpsWithFeatures.first, labels);

            }).filter([](GPSWithMatAndCapture gpsWithLabels) -> bool {

                return gpsWithLabels.second.rows != 0;

            }).filter([](GPSWithMatAndCapture gpsWithLabels) -> bool {

                auto labels = gpsWithLabels.second.row(0);
                auto gps_coord = gpsWithLabels.first.first;

                vector<int> l(labels.ptr<int>(0), labels.ptr<int>(0) + labels.cols);

                auto is_ph_label_present = std::find(l.begin(), l.end(), 1) != l.end() ||
                                           std::find(l.begin(), l.end(), 2) != l.end();

                if (is_ph_label_present) {
                    std::cout << "PH found from Camera @ [" <<
                              gps_coord.longitude << "," << gps_coord.latitude
                              << "]" << std::endl;
                } else {
                    std::cout << "NO PH found from camera @ [" <<
                              gps_coord.longitude << "," << gps_coord.latitude
                              << "]" << std::endl;
                }

                return is_ph_label_present;

            }).map([&posCapturesSaveLocation, &counter](GPSWithMatAndCapture gpsWithLabels) -> GPSCoordinates {

                auto labels = gpsWithLabels.second.row(0);
                auto capture = gpsWithLabels.first.second;
                auto gps_coord = gpsWithLabels.first.first;
                auto hash_coord_lng = std::hash<std::string>{}(std::to_string(gps_coord.longitude));
                auto hash_coord_lat = std::hash<std::string>{}(std::to_string(gps_coord.latitude));

                auto img_path = posCapturesSaveLocation +
                        "/GPS" + to_string(hash_coord_lng)  +
                        "_" + to_string(hash_coord_lat) +
                        "_I" + to_string(counter++) + ".bmp";

                cv::imwrite(img_path, capture);

                return gps_coord;

            }).subscribe(
                    subscription,
                    [subscription, serverConfig, dataTransferringNotificationLed](GPSCoordinates coordinates){
                        std::string position = toJSON(coordinates, fingerprint::getUID());

                        dataTransferringNotificationLed->switchOn();
                        sendDataToServerAsync(position, serverConfig);
                        dataTransferringNotificationLed->switchOff();

                    }, []() {
                        std::cout << "Camera Image Classifier has COMPLETED." << std::endl;
                    }
            );

            return subscription;
        }
    }
}