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
#include <random>

#include <execution/utils.h>

class ThreadSafeUpdater {
public:
    ThreadSafeUpdater () = default;

    void update(const double updatedValue){
        //Acquire the mutex and automatic release when exit from this scope
        std::lock_guard <std::mutex> lock(internal_mutex);

        internal_store = updatedValue;
    }

    double fetch(){
        //Acquire the mutex and automatic release when exit from this scope
        std::lock_guard <std::mutex> lock(internal_mutex);
        return internal_store;
    }

private:
    double internal_store;
    std::mutex internal_mutex;
};

void testA() {
    
    bool poison_pill = false;
    auto container = new ThreadSafeUpdater();

    auto t = std::thread([&]() {
        double f = 0.0;
        while(!poison_pill) {
            container->update(f);
            f = f + 1.0;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000L));
        }
    });

    auto period = std::chrono::milliseconds(1500L);

    rxcpp::connectable_observable<long> values =
            rxcpp::observable<>::interval(period, rxcpp::observe_on_event_loop())
            .publish();

    values.connect();

    values.map([&](const long v) {
        auto d = container->fetch();
        return std::pair<double, size_t>(d, sizeof(d));
    })
    .as_blocking()
    .subscribe([](const std::pair<double, size_t> l){
        printf("[B] size of %2f is %lld Byte\n", l.first, l.second);
    }, [&](){
        printf("OnCompleted\n");
        poison_pill = true;
    });

    t.join();
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

void testC() {

    std::mt19937 rng;
    rng.seed(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(0, 100);

    auto period_A = std::chrono::milliseconds(10);

    auto asPairs = [](long a, vector<long> b) {
        return std::make_pair(a, b);
    };

    auto values_A = rxcpp::observable<>::interval(period_A, rxcpp::observe_on_event_loop());

    auto A_with_B = values_A.buffer(30).map([&](std::vector<long> v){
        return asPairs(static_cast<long>(dist6(rng)), v);
    }).publish();

    A_with_B.connect();

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

    A_with_B.map([&](std::pair<long, std::vector<long>> t){
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