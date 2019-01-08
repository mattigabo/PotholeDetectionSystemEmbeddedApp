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

#include <phdetection/ontologies.hpp>
#include <phdetection/core.hpp>
#include <phdetection/svm.hpp>
#include <phdetection/bayes.hpp>

#include <fingerprint.h>

#include <accelerometer/features.h>
#include <accelerometer/utils.h>
#include <accelerometer/accelerometer.h>

#include <execution/utils.h>
#include <execution/observables/gps.h>
#include <execution/observables/accelerometer.h>

#include <serialport/SerialPort.h>
#include <serialport/SigrokSerialPortWrapper.h>

#include <gps/GPSDataStore.h>
#include <gps/GPSDataUpdater.h>

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

            void testGPS(int argc, char *argv[], string serialPortName, bool withoutRx){
                cout << "Testing the gps" << endl;

                auto gpsDataStore = new phd::devices::gps::GPSDataStore();
                phd::devices::gps::GPSDataUpdater* updater;

                phd::devices::serialport::SerialPort *serialPort = nullptr;

                auto mockedMode = false;
                if(argc >= 4) {
                    mockedMode = std::string(argv[3]) == "-mocked" || std::string(argv[2]) == "-mocked";
                } else if(argc >= 3){
                    mockedMode = std::string(argv[2]) == "-mocked";
                }

                try {
                    if (mockedMode) {
                        std::cout << "Mocked gps mode " << endl;
                        updater = new phd::devices::gps::SimulatedGPSDataUpdater(gpsDataStore);
                    } else {
                        std::cout << "Real gps mode " << endl;
                        serialPort = new phd::devices::serialport::SigrokSerialPortWrapper(serialPortName);
                        serialPort->openPort(phd::devices::serialport::READ);

                        updater = new phd::devices::gps::GPSDataUpdater(gpsDataStore, serialPort);
                    }

                    if (withoutRx) {
                        std::cout << "Execution withOUT Reactive Extensions..." << std::endl;
                        phd::test::gps::testGPSWithoutRxCpp(gpsDataStore);
                    } else {
                        std::cout << "Execution with Reactive Extensions..." << std::endl;
                        phd::test::gps::testGPSWithRxCpp(gpsDataStore);
                    }

                    updater->kill();
                    updater->join();
                    delete (updater);
                    delete (gpsDataStore);

                    if (serialPort != nullptr) {
                        serialPort->closePort();
                        delete (serialPort);
                    }
                }  catch (const string msg) {
                    cerr << msg << endl;
                }
            }
        }

        namespace accelerometer{

            void printValues(phd::devices::accelerometer::Acceleration values){
                phd::devices::accelerometer::utils::printAccelerationValues(values, "g");
                phd::devices::accelerometer::utils::printAccelerationValues(
                        phd::devices::accelerometer::utils::convertToMSSquared(values),
                        "m/s^2");
                cout << "----------------------------------------------" << endl;
            }

            void testAccelerometerWithoutRxCpp(phd::devices::accelerometer::Accelerometer *accelerometer){
                for(int i = 0; i < 20;  i++){
                    phd::devices::accelerometer::Acceleration values = accelerometer->fetch();
                    printValues(values);
                    std::this_thread::sleep_for(chrono::milliseconds(1000));
                }
            }

            void testAccelerometerWithRxCpp(phd::devices::accelerometer::Accelerometer *accelerometer){
                auto accelerationStream = observables::accelerometer::createAccelerometerObservable(
                        accelerometer,
                        observables::accelerometer::REFRESH_PERIOD_AT_2Hz
                );
                accelerationStream.as_blocking().subscribe([](const phd::devices::accelerometer::Acceleration values) {
                    printValues(values);
                },[](){
                    cout << "OnCompleted\n" << endl;
                });
            }

            void loadFeatureFromDataSet(string set,
                    std::function<int(int)> sliding_function,
                    std::vector<phd::devices::accelerometer::data::Features> &features,
                    std::vector<int> &labels){

                if (phd::io::is_dir(set.data())) {
                    vector<cv::String> globs;
                    cv::glob(set + "/*.json", globs);

                    for (const string ds : globs) {
                        const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(ds);
                        phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);
                    }

                } else if (phd::io::is_file(set.data())) {

                    const auto rawData = phd::devices::accelerometer::utils::readJSONDataset(set);
                    phd::devices::accelerometer::utils::toFeatures(rawData, "z", sliding_function, features, labels);

                } else {
                    cerr << "Undefined directory or file " << set << endl;
                    exit(-3);
                }
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

            void trainAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args,
                                               const bool cross_validate) {

                std::vector<phd::devices::accelerometer::data::Features> features;
                std::vector<int> labels;

                auto sliding_function = [](int window) { return window - 1; };

                loadFeatureFromDataSet(args.train_set, sliding_function, features, labels);

                const cv::Mat train_data = toMat(features);
                const cv::Mat normalized_train_data =
                        phd::devices::accelerometer::data::normalize(
                                train_data,
                                args.norm_range.first,
                                args.norm_range.second,
                                args.norm_method
                        );

                if (cross_validate) {
                    phd::devices::accelerometer::data::cross_train(
                            normalized_train_data,
                            cv::Mat(cv::Size(1, static_cast<int>(labels.size())), CV_32SC1, labels.data()),
                            args.model,
                            args.params.second
                    );

                } else {
                    phd::devices::accelerometer::data::train(
                            normalized_train_data,
                            cv::Mat(cv::Size(1, static_cast<int>(labels.size())), CV_32SC1, labels.data()),
                            args.model,
                            args.params.second
                    );
                }

                features.clear();
                labels.clear();
            }

            void testAccelerometerMlAlgorithm(const phd::configurations::MLOptions<phd::configurations::SVMParams> &args) {

                cout << "Testing Classifier against Test Set..." << endl;

                std::vector<phd::devices::accelerometer::data::Features> features;
                std::vector<int> labels;

//                auto sliding_function = [](int window) { return window - 1; };
                auto sliding_function = [](int window) { return window / 2; };

                loadFeatureFromDataSet(args.test_set, sliding_function, features, labels);

                const cv::Mat test_data = phd::devices::accelerometer::data::toMat(features);

                const cv::Mat normalized_test_data =
                        phd::devices::accelerometer::data::normalize(
                                test_data,
                                args.norm_range.first,
                                args.norm_range.second,
                                args.norm_method
                        );

                auto test_labels = phd::devices::accelerometer::data::classify(normalized_test_data, args.model);
                float tp = 0, fp = 0, fn = 0, tn = 0;

                for (int i = 0; i < labels.size(); ++i) {
                    if (test_labels.at<float>(0, i) == 1 && labels[i] == 1) {
                        tp++;
                    } else if (test_labels.at<float>(0, i) == 0 && labels[i] == 0) {
                        tn++;
                    } else if (test_labels.at<float>(0, i) == 1 && labels[i] == 0) {
                        fp++;
                    } else {
                        fn++;
                    }
                }

                cout << "TP: " << tp << " | TN: " << tn << " | FP: " << fp << " | FN: " << fn << endl;

                cout << "Precision: " << (tp/(tp+fp)) << endl;
                cout << "Recall/Sensitivity: " << (tp/(tp+fn)) << endl;
                cout << "F1: " << (2*tp/(2*tp+fp+fn)) << endl;
            }
        }

        namespace network{
            void testHTTPCommunication(phd::configurations::ServerConfig serverConfig){
                phd::devices::gps::Coordinates pointNearUniversity = {44.147618, 12.235476, 0};
                sendDataToServer(toJSON(pointNearUniversity, std::string()), serverConfig);
            }
        }

        namespace led {
            void testLed(phd::devices::raspberry::led::NotificationLeds notificationLeds){
                cout << "Test LED that notify the program execution" << endl;
                notificationLeds.programInExecution.switchOn();
                std::this_thread::sleep_for(1s);
                notificationLeds.programInExecution.switchOff();
                std::this_thread::sleep_for(1s);

                cout << "Test LED that notify the valid gps data" << endl;
                notificationLeds.validGpsData.switchOn();
                std::this_thread::sleep_for(1s);
                notificationLeds.validGpsData.switchOff();
                std::this_thread::sleep_for(1s);

                cout << "Test LED that notify that the data is being transferred to the server" << endl;
                notificationLeds.serverDataTransfering.switchOn();
                std::this_thread::sleep_for(1s);
                notificationLeds.serverDataTransfering.switchOff();
                std::this_thread::sleep_for(1s);

                cout << "Test LED that notify that the camera is taking a picture" << endl;
                notificationLeds.cameraIsShooting.switchOn();
                std::this_thread::sleep_for(1s);
                notificationLeds.cameraIsShooting.switchOff();
                std::this_thread::sleep_for(1s);
            }

        }

        namespace fingerprint{
            void testFingerPrintCalculation(){

                std::string uid = ::fingerprint::getUID();

                std::cout << "Fp: " << uid << std::endl;

                std::cout << "Validation: " << ::fingerprint::validateUID(uid) << std::endl;
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

                auto A_with_B = values_A.buffer(6, -3).map([&](std::vector<long> v){
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
                    std::cout << s << std::endl;
                },[](){
                    printf("OnCompleted\n");
                });
            }
        }
    }
}