//
// Created by Matteo Gabellini on 2018-12-29.
//
#include <execution/observers/gps.h>
#include <execution/observables/gps.h>

namespace observers{
    namespace gps{
        /**
         * Run gps data observer that check the gps validity and switch on the notification led when data is valid
         * and switch off it otherwise
         * */
        void runGpsValueChecker(phd::devices::gps::GPSDataStore *gpsDataStore,
                                phd::devices::raspberry::led::Led *validGpsDataNotificationLed){
            auto gps_obs = observables::gps::createGPSObservable(gpsDataStore, 1000L);
            gps_obs.as_blocking().subscribe([validGpsDataNotificationLed](phd::devices::gps::Coordinates coordinates){
                if(phd::devices::gps::coordinatesIsEqual(coordinates,phd::devices::gps::Coordinates{NAN, NAN, NAN})){
                    validGpsDataNotificationLed->switchOn();
                } else {
                    validGpsDataNotificationLed->switchOff();
                }
            });
        }
    }
}
