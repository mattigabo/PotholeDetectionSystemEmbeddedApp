//
// Created by Matteo Gabellini on 06/10/2018.
//

#include "gps/GPSDataStore.h"

namespace phd::devices::gps {

    GPSDataStore::GPSDataStore() {
        internalStore = Coordinates{0.0, 0.0, 0.0};
    }

    void GPSDataStore::update(const Coordinates updatedValue) {
        //Acquire the mutex and automatic release when exit from thi scope
        std::lock_guard <std::mutex> lock(internalMutex);

        internalStore = updatedValue;
    }

    Coordinates GPSDataStore::fetch() {
        //Acquire the mutex and automatic release when exit from thi scope
        std::lock_guard <std::mutex> lock(internalMutex);
        return internalStore;
    }
}