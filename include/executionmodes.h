//
// Created by Matteo Gabellini on 27/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H

#include <phdetection/io.hpp>

#include <gps/GPSDataStore.h>
#include <configurationutils.h>
#include <raspberrypi/Led.h>

void runObservationMode(bool poison_pill,
        phd::devices::gps::GPSDataStore* gpsDataStore,
        phd::io::Configuration phdConfig,
        phd::configurations::CVArgs cvConfig,
        phd::configurations::ServerConfig serverConfig);

void testGPSWithoutRxCpp(phd::devices::gps::GPSDataStore* storage);
void testGPSWithRxCpp(phd::devices::gps::GPSDataStore* storage);

void testHTTPCommunication(phd::configurations::ServerConfig serverConfig);

void testLed(phd::devices::raspberry::led::NotificationLeds notificationLeds);

void testAccelerometerCommunication(bool withoutRx);

void testFingerPrintCalculation();

void trainAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args,
                                   const bool cross_validate);

void testAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args);

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_EXECUTIONMODES_H
