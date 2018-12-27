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


#include <accelerometer/features.h>
#include <accelerometer/utils.h>
#include <accelerometer/accelerometer.h>

#include <execution/utils.h>
#include <execution/observables/gps.h>
#include <execution/observables/accelerometer.h>

using namespace std;
namespace phd{
    namespace test{
        namespace gps{
            void testGPSWithoutRxCpp(phd::devices::gps::GPSDataStore* storage){
                for(int i=0; i < 100; i++) {
                    phd::devices::gps::Coordinates coordinates = storage->fetch();
                    cout << "LATITUDE: " << coordinates.latitude <<
                         " LONGITUDE:" << coordinates.longitude <<
                         " ALTITUDE: " << coordinates.altitude << endl;
                    std::this_thread::sleep_for(1s);
                }
            }

            void testGPSWithRxCpp(phd::devices::gps::GPSDataStore* storage){
                auto src = observables::gps::createGPSObservable(storage, observables::gps::GPS_REFRESH_PERIOD);

                src.as_blocking().subscribe([](phd::devices::gps::Coordinates c) {
                    std::cout << c.longitude << "|" << c.latitude << std::endl;
                });
            }
        }

        namespace accelerometer{
            void testAccelerometerWithoutRxCpp(phd::devices::accelerometer::Accelerometer *accelerometer){
                for(int i = 0; i < 20;  i++){
                    phd::devices::accelerometer::Acceleration values = accelerometer->fetch();
                    cout << "Accelerometer: [ " <<
                         values.X << " on X,  " <<
                         values.Y << " on Y,  " <<
                         values.Z << " on Z ]"  << endl;
                    std::this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }

            void testAccelerometerWithRxCpp(phd::devices::accelerometer::Accelerometer *accelerometer){
                auto accelerationStream = observables::accelerometer::createAccelerometerValuesStream(
                        accelerometer,
                        observables::accelerometer::ACCELEROMETER_REFRESH_PERIOD
                );
                accelerationStream.as_blocking().subscribe([](const phd::devices::accelerometer::Acceleration values) {
                    cout << "Accelerometer: [ " <<
                         values.X << " on X,  " <<
                         values.Y << " on Y,  " <<
                         values.Z << " on Z ]"  << endl;
                },[](){
                    cout << "OnCompleted\n" << endl;
                });
            }

            void testAccelerometerCommunication(bool withoutRx){
                cout << "Test the read from the Nunchuck Accelerometer..." << endl;
                auto accelerometer = new phd::devices::accelerometer::Accelerometer();

                if(withoutRx) {
                    cout << "without RxCpp" << endl;
                    testAccelerometerWithoutRxCpp(accelerometer);
                } else {
                    cout << "with RxCpp" << endl;
                    testAccelerometerWithRxCpp(accelerometer);
                }

                //std::this_thread::sleep_for(chrono::milliseconds(5000));

                free(accelerometer);
                cout << "-----------------------------------------------" << endl;
                cout << "Bye Bye" << endl;
            }
        }

        namespace network{
            void testHTTPCommunication(phd::configurations::ServerConfig serverConfig){
                phd::devices::gps::Coordinates pointNearUniversity = {44.147618, 12.235476, 0};
                sendDataToServer(toJSON(pointNearUniversity, std::string()), serverConfig);
            }
        }

        namespace rx{
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

            void testZip() {

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

            void testBufferValues() {

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
        }
    }
}