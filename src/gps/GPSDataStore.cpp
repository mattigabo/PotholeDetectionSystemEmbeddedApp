//
// Created by Matteo Gabellini on 06/10/2018.
//

#include "gps/GPSDataStore.h"
#include <cmath>

namespace phd::devices::gps {

    GPSDataStore::GPSDataStore() {
        internal_store = Coordinates{NAN, NAN, NAN};
    }

    void GPSDataStore::update(const Coordinates updatedValue) {
        //Acquire the mutex and automatic release when exit from thi scope
        std::lock_guard <std::mutex> lock(internal_mutex);

        internal_store = updatedValue;
    }

    Coordinates GPSDataStore::fetch() {
        //Acquire the mutex and automatic release when exit from thi scope
        std::lock_guard <std::mutex> lock(internal_mutex);
        return internal_store;
    }
}