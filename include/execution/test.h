//
// Created by Xander on 04/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_TEST_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_TEST_H

#include <gps/GPSDataStore.h>
#include <raspberrypi/Led.h>

#include <networking.h>

#include "rxcpp/rx.hpp"

namespace phd{
    namespace test{
        namespace gps{
            void testGPSWithoutRxCpp(phd::devices::gps::GPSDataStore* storage);
            void testGPSWithRxCpp(phd::devices::gps::GPSDataStore* storage);
        }

        namespace accelerometer{
            void testAccelerometerCommunication(bool withoutRx);
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
