//
// Created by Matteo Gabellini on 2018-12-15.
//
#include "accelerometer/accelerometer.h"

using namespace phd::raspberry::utils;
using namespace nunchuckadapter;

namespace phd {
    namespace devices {
        namespace accelerometer{
            Accelerometer::Accelerometer() {
                setupWiringPiIfNotInitialized();
                this->reader = new NunchuckReader(NunchuckReader::InitializationMode::NOT_ENCRYPTED);
                this->dataStore = new NunchuckDataStore();
                this->dataSampler = new NunchuckDataSampler(this->reader, this-> dataStore);
            }

            Accelerometer::~Accelerometer() {
                this->dataSampler->notifyStop();
                this->dataSampler->join();

                free(this->dataSampler);
                free(this->dataStore);
                free(this->reader);
            }

            Acceleration Accelerometer::fetch() {
                NunchuckData values = dataStore->fetch();
                Acceleration result = {
                        (float)values.getAccelerationValues().X,
                        (float)values.getAccelerationValues().Y,
                        (float)values.getAccelerationValues().Z
                };
                return result;
            }
        }
    }
}
