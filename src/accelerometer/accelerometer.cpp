//
// Created by Matteo Gabellini on 2018-12-15.
//
#include "accelerometer/accelerometer.h"
#include "accelerometer/utils.h"

using namespace phd::raspberry::utils;
using namespace nunchuckadapter;

namespace phd {
    namespace devices {
        namespace accelerometer{
            Accelerometer::Accelerometer() {
                setupWiringPiIfNotInitialized();
                this->reader = new NunchuckReader(NunchuckReader::InitializationMode::NOT_ENCRYPTED);
                this->dataStore = new NunchuckDataStore();
                this->dataSampler = new NunchuckDataSampler(this->reader, this->dataStore);
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
                        values.getAccelerationValues().X,
                        values.getAccelerationValues().Y,
                        values.getAccelerationValues().Z
                };
                return result;
            }

            SimulatedAccelerometer::SimulatedAccelerometer(const std::string &path_to_acceleration_file){
                this->currentElement = 0;
                this->mockedAccelerationStream = phd::devices::accelerometer::utils::readAccelerationsInGFromDataSet(
                        path_to_acceleration_file);
            }

            Acceleration SimulatedAccelerometer::fetch() {
                if(this->currentElement >= this->mockedAccelerationStream.size()){
                    this->currentElement = 0;
                }
                Acceleration res = this->mockedAccelerationStream[this->currentElement];
                this->currentElement++;
                return res;
            }

        }
    }
}
