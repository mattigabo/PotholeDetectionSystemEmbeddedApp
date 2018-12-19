//
// Created by Xander on 04/12/2018.
//

#include "execution/observables/camera.h"

namespace observables {
    namespace  camera {

        rxcpp::observable<GPSWithMat>
        createCameraObservable(const rxcpp::observable<phd::devices::gps::Coordinates> &coordinates) {

            return coordinates.map([](phd::devices::gps::Coordinates c) {
                auto image = phd::devices::camera::fetch(cv::VideoCaptureAPIs::CAP_ANY);
                return std::pair<phd::devices::gps::Coordinates, cv::Mat>(c, image);
            });

        }
    }
}