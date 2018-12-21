//
// Created by Matteo Gabellini on 2018-12-15.
//
#include "execution/observables/accelerometer.h"

namespace observables{
    namespace accelerometer{
        rxcpp::observable<phd::devices::accelerometer::Acceleration>
        createAccelerometerValuesStream(phd::devices::accelerometer::Accelerometer *source, const long refresh){
            auto period = std::chrono::milliseconds(refresh);
            rxcpp::connectable_observable<long> streamDataGenerationClock =
                    rxcpp::observable<>::interval(period, rxcpp::observe_on_event_loop()).publish();

            streamDataGenerationClock.connect();

            auto accelerationValuesStream = streamDataGenerationClock.map([source](long v){
                phd::devices::accelerometer::Acceleration acceleration = source->fetch();
                return acceleration;
            });

            return accelerationValuesStream;
        }
    }
}
