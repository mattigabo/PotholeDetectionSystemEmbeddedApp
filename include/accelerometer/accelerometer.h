//
// Created by Matteo Gabellini on 2018-12-10.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELERATION_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELERATION_H

#include <raspberrypi/raspberrypiutils.h>

#include <nunchuckdata.h>
#include <nunchuckreader.h>
#include <nunchuckdatasampler.h>

namespace phd {
    namespace devices {
        namespace accelerometer {

            struct Acceleration {
                float X;
                float Y;
                float Z;
            };

            class Accelerometer{
            public:
                Accelerometer();
                ~Accelerometer();
                Acceleration fetch();
            private:
                nunchuckadapter::NunchuckReader* reader;
                nunchuckadapter::NunchuckDataStore* dataStore;
                nunchuckadapter::NunchuckDataSampler* dataSampler;
            };
        }
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELERATION_H
