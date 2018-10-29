//
// Created by Matteo Gabellini on 06/10/2018.
//

#ifndef POTHOLEDETECTIONEMBEDDEDAPP_GPSDATASTORE_H
#define POTHOLEDETECTIONEMBEDDEDAPP_GPSDATASTORE_H

#include <mutex>

namespace phd::devices::gps {
    typedef struct Coordinates {
        double latitude;
        double longitude;
        double altitude;
    } Coordinates;

    /**
     * This class offers a thread safe storage to store and read GPS data
     * */
    class GPSDataStore {
    public:
        GPSDataStore();

        void update(const Coordinates updatedValue);

        Coordinates fetch();

    private:
        Coordinates internal_store;
        std::mutex internal_mutex;
    };

}
#endif //POTHOLEDETECTIONEMBEDDEDAPP_GPSDATASTORE_H
