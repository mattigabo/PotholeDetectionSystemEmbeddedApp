//
// Created by Xander on 04/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVABLES_CAMERA_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVABLES_CAMERA_H

#include "rxcpp/rx.hpp"

#include <chrono>
#include <vector>
#include <string>
#include <thread>
#include <time.h>

#include <phdetection/ontologies.hpp>
#include <phdetection/core.hpp>
#include <phdetection/svm.hpp>
#include <phdetection/bayes.hpp>

#include <camera.h>
#include <gps/GPSDataStore.h>

#include "raspberrypi/led.h"

namespace observables {
    namespace  camera {

        typedef std::pair<phd::devices::gps::Coordinates, cv::Mat> GPSWithMat;

        /**
        *  Create an observable source of pairs of GPS coordinates and relative captured image
        *  from the given source of GPS coordinates
        *
        * @param coordinates The observable source of GPS coordinates
        * @return An Observable source of pair of GPS coordinates and image
        */
        rxcpp::observable<GPSWithMat>
        createCameraObservable(const rxcpp::observable<phd::devices::gps::Coordinates> &coordinates,  phd::devices::raspberry::led::Led *cameraIsShootingLed);
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_CAMERA_H
