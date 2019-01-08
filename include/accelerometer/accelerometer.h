//
// Created by Matteo Gabellini on 2018-12-10.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELERATION_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELERATION_H

#include <raspberrypi/raspberrypiutils.h>

#include <nunchuckdata.h>
#include <nunchuckreader.h>
#include <nunchuckdatasampler.h>

#include <vector>

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
                virtual Acceleration fetch();
            private:
                nunchuckadapter::NunchuckReader* reader;
                nunchuckadapter::NunchuckDataStore* dataStore;
                nunchuckadapter::NunchuckDataSampler* dataSampler;
            };

            class SimulatedAccelerometer : public Accelerometer {
            public:
                explicit SimulatedAccelerometer(const std::string &path_to_acceleration_file);
                Acceleration fetch() override;
            private:
                int currentElement;
                std::vector<Acceleration> mockedAccelerationStream;
            };
        }
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_ACCELERATION_H
