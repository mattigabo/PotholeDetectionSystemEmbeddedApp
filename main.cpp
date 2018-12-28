#include <iostream>
#include <fstream>
#include <thread>

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
#include <execution/test.h>
#include <execution/utils.h>

#include <fingerprint.h>

using namespace std;
using namespace phd::io;
using namespace phd::devices::networking;
using namespace phd::devices::serialport;
using namespace phd::devices::gps;
using namespace phd::devices::raspberry::led;
using namespace phd::configurations;

typedef Configuration PhDConfig;

void showHelper(void) {
    cout << "Execution modes" << endl;
    cout << "-o [== Run Observation process on the RasPi Camera]" << endl;
    cout << "-gps [== Test the gps communication] " <<
            "[-withoutRx = to test gps data reading without the use of RxCpp Functions] " <<
            "[-mocked = use the simulated implementation of the gps updater]" << endl;
    cout << "-http [== Test HTTP communication]" << endl;
    cout << "-led [== Test LED]" << endl;
    cout << "-accelerometer [== Test Accelerometer]" <<
            "[-withoutRx = to test accelerometer data reading without the use of RxCpp Functions]" << endl;
    cout << "-train <config-file> [ == Train the SVM classifier for the acceleration data against the given train-set(s) and test-set(s)]" << endl;
    cout << "-cross-train <config-file> [ == Cross-fold validate and Train the SVM classifier for the acceleration data against the given train-set(s) and test-set(s)]" << endl;
    cout << "-test [ == Test the trained SVM classifier for the accelerometer against the given test-set]" << endl;
    cout << "-observers [ == Test the observers]" << endl;
}

SerialPort* initSerialPort(string portName){
    SerialPort* sp = new SigrokSerialPortWrapper(portName);
    sp->openPort(READ);
    return sp;
}

void initCURL(){
    CURLcode initResult =  HTTP::init();
    cout << "cURL Global Initialization: ";
    if(initResult == CURLE_OK){
        cout << "OK";
    } else {
        cout << "Error " << initResult;
    }
    cout << "\n"<< endl;
}

void testGPS(int argc, char *argv[], string &config_folder, bool withoutRx){
    cout << "Testing the gps" << endl;

    auto gpsDataStore = new GPSDataStore();
    GPSDataUpdater* updater;

    string serialPortName;
    SerialPort *serialPort = nullptr;

    auto mockedMode = false;
    if(argc >= 4) {
        mockedMode = std::string(argv[3]) == "-mocked" || std::string(argv[2]) == "-mocked";
    } else if(argc >= 3){
        mockedMode = std::string(argv[2]) == "-mocked";
    }

    if(mockedMode){
        updater = new phd::devices::gps::SimulatedGPSDataUpdater(gpsDataStore);
    } else {
        serialPortName = loadSerialPortFromConfig(config_folder + "/config.json");
        serialPort = initSerialPort(serialPortName);

        updater = new phd::devices::gps::GPSDataUpdater(gpsDataStore, serialPort);
    }

    if(withoutRx){
        std::cout << "Mocked mode withOUT Reactive Extensions..." << std::endl;
        phd::test::gps::testGPSWithoutRxCpp(gpsDataStore);
    } else {
        std::cout << "Mocked mode with Reactive Extensions..." << std::endl;
        phd::test::gps::testGPSWithRxCpp(gpsDataStore);
    }

    updater->kill();
    updater->join();
    delete (updater);
    delete (gpsDataStore);

    if(serialPort != nullptr) {
        serialPort->closePort();
        delete (serialPort);
    }
}

void testObservers(CVArgs &cvConfig, PhDConfig &phdConfig, MLOptions<SVMParams> &svmConfig, ServerConfig &serverConfig){

    auto gpsDataStore = new GPSDataStore();
    auto updater = new phd::devices::gps::SimulatedGPSDataUpdater(gpsDataStore);
    auto accelerometer = new phd::devices::accelerometer::Accelerometer();
    auto axis = phd::devices::accelerometer::data::Axis::Z;

    std::cout << "RUNNING RX Accelerometer data stream classification." << std::endl;
    observers::accelerometer::runAccelerometerObserver(
            gpsDataStore,
            accelerometer,
            axis,
            phdConfig,
            svmConfig,
            serverConfig
    );

    std::cout << "RUNNING RX Camera data stream classification." << std::endl;
    observers::camera::runCameraObserver(
            gpsDataStore,
            phdConfig,
            cvConfig,
            serverConfig
    );

    observables::gps::createGPSObservable(gpsDataStore, 2000L).as_blocking().subscribe();

    updater->kill();
    updater->join();
    delete (updater);
    delete (gpsDataStore);
}

int main(int argc, char *argv[]) {

//    cout << phd::io::GetCurrentWorkingDir() << endl;

    const string config_folder_suffix = "/res/config";

    const NotificationLeds notificationLeds = { Led(0), Led(1), Led(2), Led(3)};

    const string root = phd::io::getParentDirectory(string(dirname(argv[0])));

    auto config_folder = root + config_folder_suffix;
    cout << config_folder << endl;

    initCURL();

    if (argc < 2) {
        showHelper();

        return 0;
    } else {

        auto mode = std::string(argv[1]);
        auto withoutRx = false;
        if(argc > 2){
            withoutRx = std::string(argv[2]) == "-withoutRx";
        }

        cout << mode << " mode is ACTIVE." << endl << endl;

        auto poison_pill = false;

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

        if (mode == "-http") {

            phd::test::network::testHTTPCommunication(serverConfig);

        } else if (mode == "-led") {

            phd::test::led::testLed(notificationLeds);

        } else if (mode == "-accelerometer") {

            phd::test::accelerometer::testAccelerometerCommunication(withoutRx);

        } else if (mode == "-train" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);
            phd::test::accelerometer::trainAccelerometerMlAlgorithm(svmConfig, false);

        } else if (mode == "-cross-train" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);

            phd::test::accelerometer::trainAccelerometerMlAlgorithm(svmConfig, true);
            phd::test::accelerometer::testAccelerometerMlAlgorithm(svmConfig);

        } else if (mode == "-test" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);

            phd::test::accelerometer::testAccelerometerMlAlgorithm(svmConfig);

        } else if (mode == "-observers") {

            testObservers(cvArgs, phdConfig, svmConfig, serverConfig);

        } else if (mode == "-fp") {

            phd::test::fingerprint::testFingerPrintCalculation();

        } else if (mode == "-gps"){

            testGPS(argc, argv, config_folder, withoutRx);

        } else if (mode == "-o") {

            const string fp = fingerprint::getUID();

            cout << "Registering Device " << fp <<" on Server..." << endl;

            registerDeviceOnServer(toJSON(fp), serverConfig);

            string serialPortName = loadSerialPortFromConfig(config_folder + "/config.json");
            SerialPort *serialPort = initSerialPort(serialPortName);

            auto gpsDataStore = new GPSDataStore();
            auto updater = new phd::devices::gps::GPSDataUpdater(gpsDataStore, serialPort);
            auto accelerometer = new phd::devices::accelerometer::Accelerometer();
            auto axis = phd::devices::accelerometer::data::Axis::Z;

//            runObservationMode(poison_pill, gpsDataStore, phdConfig, cvArgs, serverConfig);

//            observers::camera::runCameraObserver(gpsDataStore, phdConfig, cvArgs, serverConfig);

            observers::accelerometer::runAccelerometerObserver(
                    gpsDataStore,
                    accelerometer,
                    axis,
                    phdConfig,
                    svmConfig,
                    serverConfig
                );

            updater->kill();
            updater->join();
            delete (updater);
            delete (gpsDataStore);

            serialPort->closePort();
            delete (serialPort);
        } else {
            showHelper();
        }
    }

    HTTP::close();
    return 1;
}