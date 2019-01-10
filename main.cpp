#include <iostream>
#include <fstream>
#include <thread>
#include <functional>
#include <libgen.h>

#include <opencv2/core.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

#include <gps/GPSDataStore.h>
#include <gps/GPSDataUpdater.h>
#include <serialport/SerialPort.h>
#include <serialport/SigrokSerialPortWrapper.h>
#include <networking.h>
#include <configurationutils.h>
#include <executionmodes.h>
#include <camera.h>
#include <accelerometer/features.h>
#include <phdetection/io.hpp>

#include <raspberrypi/raspberrypiutils.h>

#include <execution/observables/gps.h>
#include <execution/observers/accelerometer.h>
#include <execution/observers/camera.h>
#include <execution/observers/gps.h>
#include <execution/test.h>
#include <execution/utils.h>

#include <fingerprint.h>

#include <future>

using namespace std;
using namespace phd::io;
using namespace phd::devices::networking;
using namespace phd::devices::serialport;
using namespace phd::devices::gps;
using namespace phd::devices::raspberry::led;
using namespace phd::configurations;

void showHelper(void) {
    cout << "Execution modes" << endl;
    cout << "-o [== Run Observation process on the accelerometer] " <<
            "-camera [in order to observe also on the RasPi Camera]" << endl;
    cout << "-gps [== Test the gps communication] " <<
            "[-withoutRx = to test gps data reading without the use of RxCpp Functions] " <<
            "[-mocked = use the simulated implementation of the gps updater]" << endl;
    cout << "-http [== Test HTTP communication]" << endl;
    cout << "-led [== Test LED]" << endl;
    cout << "-accelerometer [== Test Accelerometer] -simulatedAcc [== use the simulated version of the accelerometer that read data from the test_set]" <<
            "[-withoutRx = to test accelerometer data reading without the use of RxCpp Functions]" << endl;
    cout << "-train <config-file> [ == Train the SVM classifier for the acceleration data against the given train-set(s) and test-set(s)]" << endl;
    cout << "-cross-train <config-file> [ == Cross-fold validate and Train the SVM classifier for the acceleration data against the given train-set(s) and test-set(s)]" << endl;
    cout << "-test [ == Test the trained SVM classifier for the accelerometer against the given test-set]" << endl;
    cout << "-observers [ == Test the observers]" <<
            "-camera [in order to observe also on the RasPi Camera]" << endl;
}

EmbeddedAppConfiguration loadConfig(string config_folder){
    cout << "Loading Computer Vision arguments..." << endl;
    CVArgs cvArgs = loadCVArgs(config_folder + "/config.json");;
    cout << "Computer Vision arguments Loaded." << endl << endl;

    cout << "Loading Computer Vision Configuration..." << endl;
    PhDConfig phdConfig = loadProgramConfiguration(config_folder + "/config.json");
    cout << "Computer Vision Configuration Loaded." << endl << endl;

    cout << "Loading Accelerometer SVM Configuration..." << endl;
    MLOptions<SVMParams> svmConfig = loadSVMOptions(config_folder + "/config.json");
    cout << "Accelerometer SVM Configuration Loaded." << endl << endl;

    cout << "Loading Server Configuration..." << endl;
    ServerConfig serverConfig = loadServerConfig(config_folder + "/config.json");
    cout << "Server Configuration Loaded." << endl << endl;

    string serialPortName = loadSerialPortFromConfig(config_folder + "/config.json");

    return {cvArgs, phdConfig, serverConfig, svmConfig, serialPortName};
}

SerialPort* initSerialPort(string portName){
    SerialPort* sp = new SigrokSerialPortWrapper(portName);
    sp->openPort(READ);
    return sp;
}

void initHTTP(){
    CURLcode initResult =  HTTP::async::init();
    cout << "HTTP Global Initialization: ";
    if(initResult == CURLE_OK){
        cout << "OK";
    } else {
        cout << "Error " << initResult;
    }
    cout << endl << endl;
}

void selectMode(int argc, char *argv[], EmbeddedAppConfiguration loadedConfig){

    auto args = parseCommandLine(argc, argv);

    cout << args.mode << " mode is ACTIVE." << endl << endl;

    NotificationLeds notificationLeds = { Led(0), Led(1), Led(2), Led(3)};
    auto gpsDataStore = new phd::devices::gps::GPSDataStore();

    if (args.mode == "-http") {

        phd::test::network::testHTTPCommunication(loadedConfig.serverConfig);

    } else if (args.mode == "-led") {

        phd::test::led::testLed(notificationLeds);

    } else if (args.mode == "-accelerometer") {
        phd::test::accelerometer::testAccelerometerCommunication(args.withoutRx, args.simulatedAccelerometer, loadedConfig);

    } else if (args.mode == "-train") {

        phd::test::accelerometer::trainAccelerometerMlAlgorithm(loadedConfig.svmConfig, false);

    } else if (args.mode == "-cross-train") {

        auto train_config = loadedConfig.svmConfig;

        if (argc > 2) {
            train_config = loadSVMOptions(std::string(argv[2]));
        }

        auto test_config = phd::test::accelerometer::trainAccelerometerMlAlgorithm(train_config, true);

        phd::test::accelerometer::testAccelerometerMlAlgorithm(test_config);

    } else if (args.mode == "-test") {

        phd::test::accelerometer::testAccelerometerMlAlgorithm(loadedConfig.svmConfig);

    }  else if (args.mode == "-fp") {

        phd::test::fingerprint::testFingerPrintCalculation();

    } else if (args.mode == "-gps"){

        phd::test::gps::testGPS(argc, argv, loadedConfig.serialPortName, args.withoutRx);

    } else if (args.mode == "-observers") {

        auto updater = new phd::devices::gps::SimulatedGPSDataUpdater(gpsDataStore);

        phd::devices::accelerometer::Accelerometer *accelerometer;
        if(args.simulatedAccelerometer) {
            cout << "Run with the Simulated Accelerometer..." << endl;
            accelerometer = new phd::devices::accelerometer::SimulatedAccelerometer(loadedConfig.svmConfig.test_set);
        } else {
            cout << "Run with Nunchuck Accelerometer..." << endl;
            accelerometer = new phd::devices::accelerometer::Accelerometer();
        }

        phd::executionmodes::runObservationMode(loadedConfig,
                                                gpsDataStore,
                                                accelerometer,
                                                notificationLeds,
                                                args);

        updater->kill();
        updater->join();
        delete (updater);

    } else if (args.mode == "-o") {
        try {
            SerialPort *serialPort = initSerialPort(loadedConfig.serialPortName);

            auto updater = new phd::devices::gps::GPSDataUpdater(gpsDataStore, serialPort);
            phd::devices::accelerometer::Accelerometer *accelerometer;
            if(args.simulatedAccelerometer) {
                cout << "Run with the Simulated Accelerometer..." << endl;
                accelerometer = new phd::devices::accelerometer::SimulatedAccelerometer(loadedConfig.svmConfig.test_set);
            } else {
                cout << "Run with Nunchuck Accelerometer..." << endl;
                accelerometer = new phd::devices::accelerometer::Accelerometer();
            }

            phd::executionmodes::runObservationMode(loadedConfig,
                                                    gpsDataStore, nullptr,
                                                    notificationLeds,
                                                    args);

            updater->kill();
            updater->join();
            delete (updater);

            serialPort->closePort();
            delete (serialPort);
        }  catch (const string msg) {
            cerr << msg << endl;
        }
    } else {
        showHelper();
    }
    delete (gpsDataStore);
}

int main(int argc, char *argv[]) {

//    cout << phd::io::GetCurrentWorkingDir() << endl;

    const string config_folder_suffix = "/res/config";

    const string root = phd::io::getParentDirectory(string(dirname(argv[0])));

    auto config_folder = root + config_folder_suffix;

    cout << config_folder << endl;

    initHTTP();

    if (argc < 2) {
        showHelper();
        return 0;
    } else {
        EmbeddedAppConfiguration loadedConfig = loadConfig(config_folder);
        selectMode(argc, argv, loadedConfig);
    }

    HTTP::async::close();

    std::cout << std::endl << "---------------------------------------------------------" << std::endl << std::endl;

    return 1;
}