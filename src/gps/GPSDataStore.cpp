//
// Created by Matteo Gabellini on 06/10/2018.
//

#include "gps/GPSDataStore.h"

namespace phd {
    namespace devices {
        namespace gps {

            bool coordinatesIsEqual(Coordinates a, Coordinates b){
                return a.latitude == b.latitude &&
                       a.longitude == b.longitude &&
                       a.altitude == b.latitude;
            }

            bool isInvalid(Coordinates cord){
                return isnan(cord.latitude) || isnan(cord.longitude) || isnan(cord.altitude);
            }

            GPSDataStore::GPSDataStore() {
                internal_store = emptyCoordinates;
            }

            void GPSDataStore::update(const Coordinates updatedValue) {
                //Acquire the mutex and automatic release when exit from this scope
                std::lock_guard <std::mutex> lock(internal_mutex);

                internal_store = updatedValue;
            }

            Coordinates GPSDataStore::fetch() {
                //Acquire the mutex and automatic release when exit from this scope
                std::lock_guard <std::mutex> lock(internal_mutex);
                return internal_store;
            }

        }
    }
}