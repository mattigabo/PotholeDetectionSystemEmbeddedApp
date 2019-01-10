//
// Created by Xander on 04/12/2018.
//

#include "execution/observables/camera.h"

#include <chrono>
#include <string>
#include <thread>
#include <time.h>

namespace observables {
    namespace  camera {

        rxcpp::observable<GPSWithMat>
        createCameraObservable(const rxcpp::observable<phd::devices::gps::Coordinates> &coordinates,  phd::devices::raspberry::led::Led *cameraIsShootingLed) {

            return coordinates.map([cameraIsShootingLed](phd::devices::gps::Coordinates c) {
                cameraIsShootingLed->switchOn();
                auto image = phd::devices::camera::fetch(cv::VideoCaptureAPIs::CAP_ANY);
                cameraIsShootingLed->switchOff();
                return std::pair<phd::devices::gps::Coordinates, cv::Mat>(c, image);
            });

        }
    }
}