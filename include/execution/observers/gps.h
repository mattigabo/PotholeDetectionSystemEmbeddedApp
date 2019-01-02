//
// Created by Matteo Gabellini on 2018-12-29.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_GPS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_GPS_H

#include <gps/GPSDataStore.h>
#include <raspberrypi/led.h>
#include <rxcpp/rx.hpp>

namespace observers{
    namespace gps{
        /**
         * Run gps data observer that check the gps validity and switch on the notification led when data is valid
         * and switch off it otherwise
         * */
        rxcpp::composite_subscription runGpsValueChecker(phd::devices::gps::GPSDataStore *gpsDataStore,
                                                         phd::devices::raspberry::led::Led *validGpsDataNotificationLed);
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBSERVERS_GPS_H
