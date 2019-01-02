//
// Created by Matteo Gabellini on 27/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H

#include <phdetection/io.hpp>

#include <configurationutils.h>
#include <raspberrypi/led.h>

#include <serialport/SerialPort.h>
#include <gps/GPSDataStore.h>

namespace phd {
    namespace executionmodes {
        void runObservationMode(phd::configurations::EmbeddedAppConfiguration loadedConfig,
                                        phd::devices::gps::GPSDataStore *gpsDataStore,
                                        phd::devices::raspberry::led::NotificationLeds notificationLeds,
                                        phd::configurations::CommandLineArgs cmdArgs);
    }
}
#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H
