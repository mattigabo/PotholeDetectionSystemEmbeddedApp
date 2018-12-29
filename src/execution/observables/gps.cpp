//
// Created by Xander on 04/12/2018.
//

#include "execution/observables/gps.h"

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

namespace observables {
    namespace gps {

        rxcpp::observable<phd::devices::gps::Coordinates>
        createGPSObservable(phd::devices::gps::GPSDataStore *gpsDataStore, const long refresh) {

            auto period = std::chrono::milliseconds(refresh);
            rxcpp::connectable_observable<long> tick_tock =
                    rxcpp::observable<>::interval(period, rxcpp::observe_on_event_loop()).publish();

            tick_tock.connect();

            return tick_tock.map([gpsDataStore](long v) {
                phd::devices::gps::Coordinates c = gpsDataStore->fetch();
                return c;
            });
        }
    }
}