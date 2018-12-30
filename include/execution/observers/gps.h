//
// Created by Matteo Gabellini on 2018-12-29.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_GPS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_GPS_H

#include <gps/GPSDataStore.h>
#include <raspberrypi/led.h>

namespace observers{
    namespace gps{
        /**
         * Run gps data observer that check the gps validity and switch on the notification led when data is valid
         * and switch off it otherwise
         * */
        void runGpsValueChecker(phd::devices::gps::GPSDataStore *gpsDataStore,
                phd::devices::raspberry::led::Led *validGpsDataNotificationLed);
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_GPS_H
