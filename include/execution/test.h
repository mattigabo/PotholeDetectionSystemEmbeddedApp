//
// Created by Xander on 04/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_TEST_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_TEST_H

#include <gps/GPSDataStore.h>
#include <raspberrypi/led.h>

#include <networking.h>

#include "rxcpp/rx.hpp"

namespace phd{

    namespace test{

        namespace gps{

            void testGPSWithoutRxCpp(phd::devices::gps::GPSDataStore* storage);

            void testGPSWithRxCpp(phd::devices::gps::GPSDataStore* storage);

            void testGPS(int argc, char *argv[], std::string serialPortName, bool withoutRx);
        }

        namespace accelerometer{

            void testAccelerometerCommunication(bool withoutRx, bool simulated, phd::configurations::EmbeddedAppConfiguration loadedConfig);

            phd::configurations::MLOptions<phd::configurations::SVMParams>
                    trainAccelerometerMlAlgorithm(
                            const phd::configurations::MLOptions<phd::configurations::SVMParams> &args,
                            const bool cross_validate
                    );

            void testAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args);
        }

        namespace network{

            void testHTTPCommunication(phd::configurations::ServerConfig serverConfig);

        }

        namespace led{

            void testLed(phd::devices::raspberry::led::NotificationLeds notificationLeds);

        }

        namespace fingerprint{

            void testFingerPrintCalculation();

        }

        namespace rx {

            void testA();

            void testZip();

            void testBufferValues();

        }
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_TEST_H
