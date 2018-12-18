//
// Created by Xander on 04/12/2018.
//

#include "execution/observables/gps.h"

namespace observables {
    namespace gps {

        rxcpp::observable<phd::devices::gps::Coordinates>
        createGPSObservable(phd::devices::gps::GPSDataStore *gpsDataStore, const long refresh) {

            auto period = std::chrono::milliseconds(refresh);
            rxcpp::connectable_observable<long> tick_tock =
                    rxcpp::observable<>::interval(period, rxcpp::observe_on_event_loop()).publish();

            tick_tock.connect();

            return tick_tock.map([gpsDataStore](long v) {
                phd::devices::gps::Coordinates c = gpsDataStore->fetch();
                return c;
            });

            //    auto gen = [&](rxcpp::subscriber<phd::devices::gps::Coordinates> s, phd::devices::gps::GPSDataStore *gps){
            //        bool poison_pill = false;
            //
            //        while (!poison_pill) {
            //            if (!s.is_subscribed()) {
            //                poison_pill = true;
            //            } else {
            //                auto coo = gps->fetch();
            //                s.on_next(coo);
            //                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
            //            }
            //        }
            //        s.on_completed();
            //    };
            //
            //    rxcpp::connectable_observable<phd::devices::gps::Coordinates> gpsObservable =
            //            rxcpp::observable<>::create<phd::devices::gps::Coordinates>(
            //                [&](rxcpp::subscriber<phd::devices::gps::Coordinates> s){
            //                    std::thread(gen, s, gpsDataStore).join();
            //                }).publish();
            //
            //    gpsObservable.connect();
            //
            //    return gpsObservable;

        }
    }
}