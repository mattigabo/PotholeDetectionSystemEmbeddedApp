//
// Created by Xander on 04/12/2018.
//

#include "execution/test.h"

#include <algorithm>
#include <numeric>
#include <execution/utils.h>

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

void testB() {

    auto period = std::chrono::milliseconds(15);

    auto values = rxcpp::observable<>::interval(period, rxcpp::observe_on_event_loop())
                    .publish();

    values.connect();

    auto buffered_values = values.buffer(30).map([&](std::vector<long> v) {
        return std::accumulate(v.begin(), v.end(), 0);
    });

    buffered_values.subscribe([&](long sum30){
        std::cout << sum30 << std::endl;
    },[](){
        printf("OnCompleted\n");
    });

    buffered_values.as_blocking().subscribe();
}