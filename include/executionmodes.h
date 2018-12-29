//
// Created by Matteo Gabellini on 27/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H

#include <phdetection/io.hpp>

#include <gps/GPSDataStore.h>
#include <configurationutils.h>
#include <raspberrypi/led.h>

void runObservationMode(bool poison_pill,
        phd::devices::gps::GPSDataStore* gpsDataStore,
        phd::io::Configuration phdConfig,
        phd::configurations::CVArgs cvConfig,
        phd::configurations::ServerConfig serverConfig);

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H
