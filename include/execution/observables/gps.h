//
// Created by Xander on 04/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVABLES_GPS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVABLES_GPS_H
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

namespace observables {
    namespace gps {

        const long GPS_REFRESH_RATE = 1500L;

        /**
        * Create an HOT Observable from the given GPSDataStore. Values are picked every refresh
        *
        * @param gpsDataStore The storage from which the GPS Coordinates are read
        * @param refresh The refresh time of the read
        * @return An observable of Coordinates
        */
        rxcpp::observable<phd::devices::gps::Coordinates>
        createGPSObservable(phd::devices::gps::GPSDataStore *gpsDataStore, const long refresh);
    }
}


#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVABLES_GPS_H
