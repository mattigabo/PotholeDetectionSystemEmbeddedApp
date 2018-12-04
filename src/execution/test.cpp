//
// Created by Xander on 04/12/2018.
//

#include "execution/test.h"

void testA() {

    auto period = std::chrono::milliseconds(1500);

    rxcpp::connectable_observable<long> values =
            rxcpp::observable<>::interval(period, rxcpp::observe_on_event_loop())
            .publish();

    values.subscribe([&](const long v) {
        printf("[C] Value: %ld\n", v);
    });

    values.connect();

    values.map([&](const long v) {
        return std::pair<double, size_t>((double)v, sizeof((double)v) * 8);
    }).subscribe([](const std::pair<double, size_t> l){
        printf("[B] size of %f is %lld\n", l.first, l.second);
    }, [](){
        printf("OnCompleted\n");
    });

    rxcpp::observable<>::timer(period, rxcpp::observe_on_event_loop())
            .subscribe([&](long){
                values.map([&](const int v) {
                    return std::pair<int, size_t>(v, sizeof(v) * 8);
                }).subscribe([](const std::pair<int, size_t> l){
                    printf("[A] size of %d is %lld\n", l.first, l.second);
                }, [](){
                    printf("OnCompleted\n");
                });
            });

    values.as_blocking().subscribe();

}