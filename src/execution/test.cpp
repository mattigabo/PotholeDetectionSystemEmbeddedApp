//
// Created by Xander on 04/12/2018.
//

#include "execution/test.h"

#include <chrono>
#include <vector>
#include <string>
#include <thread>
#include <time.h>
#include <algorithm>
#include <numeric>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <iostream>

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

    auto period_A = std::chrono::milliseconds(10);
    auto period_B = std::chrono::milliseconds(50);

    auto asPairs = [](long a, vector<long> b) {
        return std::make_pair(a, b);
    };

    auto values_A = rxcpp::observable<>::interval(period_A, rxcpp::observe_on_event_loop());

    auto values_B = rxcpp::observable<>::interval(period_B, rxcpp::observe_on_event_loop());

    auto values_AB = values_B.zip(
            rxcpp::observe_on_new_thread(), // Scheduler
            asPairs, // Zipping Function
            values_A.buffer(30) // The stream to be zipped with
        ).publish();

    values_AB.connect();

    auto vector_to_string = [&](std::vector<long> v) {
        std::ostringstream oss;
        oss << "[";
        if (!v.empty()){

            // Convert all but the last element to avoid a trailing ","
            std::copy(v.begin(), v.end()-1,
                      std::ostream_iterator<int>(oss, ","));

            // Now add the last element with no delimiter
            oss << v.back();
        }
        oss << "]";

        return oss.str();
    };

    values_AB
    .map([&](std::pair<long, std::vector<long>> t){
        return std::make_tuple(
            t.first,
            std::accumulate(t.second.begin(), t.second.end(), 0),
            vector_to_string(t.second)
        );
    })
    .as_blocking()
    .subscribe([&](std::tuple<long, long, std::string> t){

        long i, sum;
        std::string s;
        std::tie(i, sum, s) = t;

        std::cout << i << "|" << sum << "|" << s.size() << std::endl;
    },[](){
        printf("OnCompleted\n");
    });
}