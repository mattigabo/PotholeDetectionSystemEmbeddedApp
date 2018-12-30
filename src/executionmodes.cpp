//
// Created by Matteo Gabellini on 27/10/2018.
//
#include <executionmodes.h>

#include <chrono>
#include <vector>
#include <string>
#include <thread>
#include <time.h>
#include <assert.h>
#include <fstream>

#include <phdetection/ontologies.hpp>
#include <phdetection/core.hpp>
#include <phdetection/svm.hpp>
#include <phdetection/bayes.hpp>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

#include <serialport/SerialPort.h>
#include <camera.h>
#include <networking.h>
#include <accelerometer/features.h>
#include <accelerometer/utils.h>
#include <accelerometer/accelerometer.h>

#include <gps/GPSDataStore.h>
#include <gps/GPSDataUpdater.h>
#include <raspberrypi/led.h>

#include <execution/utils.h>
#include <execution/observables/accelerometer.h>
#include <execution/observables/gps.h>

#include <execution/observers/accelerometer.h>
#include <execution/observers/gps.h>
#include <execution/observers/camera.h>

#include <fingerprint.h>

using namespace rapidjson;
using namespace std;

namespace phd {
    namespace executionmodes {

        cv::Mat extractFeaturesAndClassify(const string &method, const string &bayes_model, const string &svm_model,
                                           cv::Mat &image,
                                           const phd::io::Configuration &phdConfig) {

            //cout << endl << "---------------" << image << endl;

            auto features = phd::getFeatures(image, phdConfig);

            cv::Mat labels;

            try {
                labels = phd::classify(method, svm_model, bayes_model, features);
            } catch (phd::UndefinedMethod &ex) {
                cerr << "ERROR: " << ex.what() << endl;
                exit(-1);
            }

            cout << "LABELS: " << labels << endl;

            return labels;
        }

        void doFingerprinting(phd::configurations::ServerConfig serverConfig){
            const string fp = fingerprint::getUID();

            cout << "Registering Device " << fp << " on Server..." << endl;
            //            auto f = std::async(std::launch::async, [fp, serverConfig]() {
            registerDeviceOnServer(toJSON(fp), serverConfig);
            //            }
        }

        void runObservationMode(phd::configurations::EmbeddedAppConfiguration loadedConfig,
                                devices::gps::GPSDataStore *gpsDataStore,
                                phd::devices::raspberry::led::NotificationLeds notificationLeds,
                                bool useCamera) {

            notificationLeds.programInExecution.switchOn();

            doFingerprinting(loadedConfig.serverConfig);

            auto accelerometer = new phd::devices::accelerometer::Accelerometer();
            auto axis = phd::devices::accelerometer::data::Axis::Z;

            observers::gps::runGpsValueChecker(gpsDataStore, &notificationLeds.validGpsData);

            if(useCamera) {
                std::cout << "RUNNING RX Camera data stream classification." << std::endl;
                observers::camera::runCameraObserver(gpsDataStore,
                        loadedConfig.phdConfig,
                        loadedConfig.cvConfig,
                        loadedConfig.serverConfig,
                        &notificationLeds.serverDataTransfering);
            }

            std::cout << "RUNNING RX Accelerometer data stream classification." << std::endl;
            observers::accelerometer::runAccelerometerObserver(
                    gpsDataStore,
                    accelerometer,
                    axis,
                    loadedConfig.phdConfig,
                    loadedConfig.svmConfig,
                    loadedConfig.serverConfig,
                    &notificationLeds.serverDataTransfering
            );

            notificationLeds.programInExecution.switchOff();
        }
    }
}
