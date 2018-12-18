//
// Created by Matteo Gabellini on 27/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H

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

void testGPSWithoutRxCpp(GPSDataStore* storage);
void testGPSWithRxCpp(GPSDataStore* storage);

void testHTTPCommunication(ServerConfig serverConfig);

void testLed(NotificationLeds notificationLeds);

void testAccelerometerCommunication(bool withoutRx);

void testFingerPrintCalculation();

void trainAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args,
                                   const bool cross_validate);

void testAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args);

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H
