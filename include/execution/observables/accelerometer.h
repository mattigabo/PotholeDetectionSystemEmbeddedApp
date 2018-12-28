//
// Created by Matteo Gabellini on 2018-12-15.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBESRVABLES_ACCELEROMETER_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBESRVABLES_ACCELEROMETER_H

#include <accelerometer/accelerometer.h>
#include "rxcpp/rx.hpp"

namespace observables{
    namespace accelerometer{
        const long ACCELEROMETER_REFRESH_PERIOD = 500L;

        /**
       * Create an HOT Observable stream from the given Accelerometer. Values are emitted every refresh
       *
       * @param source The storage from which the Accelerometer Values are read
       * @param refresh The refresh time of the read
       * @return An observable stream of acceleration values
       */
        rxcpp::observable<phd::devices::accelerometer::Acceleration>
        createAccelerometerValuesStream(phd::devices::accelerometer::Accelerometer *source, const long refresh);
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_OBESRVABLES_ACCELEROMETER_H
