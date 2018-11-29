//
// Created by Matteo Gabellini on 27/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_EXECUTIONMODES_H
#define POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_EXECUTIONMODES_H

#include <phdetection/io.hpp>

#include <gps/GPSDataStore.h>
#include <configurationutils.h>
#include <raspberrypi/Led.h>

using namespace phd::devices::gps;
using namespace phd::configurations;
using namespace phd::devices::raspberry::led;
using namespace phd::io;

void runObservationMode(bool poison_pill,
        GPSDataStore* gpsDataStore,
        Configuration phdConfig,
        CVArgs cvConfig,
        ServerConfig serverConfig);

void testGPSCommunication(GPSDataStore* storage);

void testHTTPCommunication(ServerConfig serverConfig);

void testLed(NotificationLeds notificationLeds);

void trainAccelerometer(const phd::configurations::SVMArgs &args);

void testAccelerometer(const phd::configurations::SVMArgs &args);

#endif //POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_EXECUTIONMODES_H
