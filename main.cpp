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
#include <accelerometer/ml.h>
#include <phdetection/io.hpp>

#include <raspberrypi/raspberrypiutils.h>

#include <execution/observables/camera.h>
#include <execution/observers/camera.h>
#include <execution/test.h>
#include <execution/utils.h>

#include <fingerprint.h>

using namespace std;
using namespace phd::io;
using namespace phd::devices::networking;
using namespace phd::devices::serialport;
using namespace phd::devices::gps;
using namespace phd::configurations;

Configuration phdConfig;
ServerConfig serverConfig;
CVArgs cvConfig;
string serialPortName;
string config_folder = "/res/config";

NotificationLeds notificationLeds = { Led(0), Led(1), Led(2), Led(3)};

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
    cout << "-train <config-file> [ == Train and Test the SVM classifier against the given train-set(s) and test-set(s)]" << endl;
    cout << "-test <config-file> [ == Trained Classify against the given test-set]" << endl;
}

SerialPort* initSerialPort(string portName){
    SerialPort* sp = new SigrokSerialPortWrapper(portName);
    sp->openPort(READ);
    return sp;
}

int main(int argc, char *argv[]) {

//    cout << phd::io::GetCurrentWorkingDir() << endl;

    const string root = phd::io::getParentDirectory(string(dirname(argv[0])));

    config_folder = root + config_folder;
    cout << config_folder << endl;

    CURLcode initResult =  HTTP::init();
    cout << "cURL Global Initialization: ";
    if(initResult == CURLE_OK){
        cout << "OK";
    } else {
        cout << "Error " << initResult;
    }
    cout << "\n"<< endl;

    if (argc < 2) {
        showHelper();

        return 0;
    } else {

        auto mode = std::string(argv[1]);
        auto withoutRx = false;
        if(argc > 2){
            withoutRx = std::string(argv[2]) == "-withoutRx";
        }

        cout << mode << " mode: ON \n" << endl;

        auto poison_pill = false;

        cout << "Loading Computer Vision Configuration..." << endl;
        phdConfig = loadProgramConfiguration(config_folder + "/config.json");
        cout << "Computer Vision Configuration Loaded" << endl;

        serverConfig = loadServerConfig(config_folder + "/config.json");
        cout << "Server Configuration Loaded\n" << endl;

        if (mode == "-http") {
            testHTTPCommunication(serverConfig);
        } else if (mode == "-led") {
            testLed(notificationLeds);
        } else if (mode == "-accelerometer") {
            testAccelerometerCommunication(withoutRx);
        } else if (mode == "-train" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);

            trainAccelerometerMlAlgorithm(svmConfig, false);

        } else if (mode == "-cross-train" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);

            trainAccelerometerMlAlgorithm(svmConfig, true);
            testAccelerometerMlAlgorithm(svmConfig);

        } else if (mode == "-test" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);
            testAccelerometerMlAlgorithm(svmConfig);

        } else if (mode == "-fp") {

            testFingerPrintCalculation();

        } else if (mode == "-gps"){

            auto gpsDataStore = new GPSDataStore();
            GPSDataUpdater* updater;
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
//                testGPSWithoutRxCpp(gpsDataStore);
                std::cout << "Mocked mode withOUT Reactive Extensions..." << std::endl;
                runObservationMode(poison_pill, gpsDataStore, phdConfig, cvConfig, serverConfig);
            } else {
                cvConfig = loadCVArgs(config_folder + "/config.json");
                std::cout << "Mocked mode with Reactive Extensions..." << std::endl;
                observers::camera::runCameraObserver(gpsDataStore, phdConfig, cvConfig, serverConfig);

//                testGPSWithRxCpp(gpsDataStore);
            }

            updater->kill();
            updater->join();
            delete (updater);
            delete (gpsDataStore);

            if(serialPort != nullptr) {
                serialPort->closePort();
                delete (serialPort);
            }

        } else if (mode == "-o") {

            cout << "Registering Device on Server..." << endl;
            registerDeviceOnServer(toJSON(fingerprint::getUID()), serverConfig);

            serialPortName = loadSerialPortFromConfig(config_folder + "/config.json");
            SerialPort *serialPort = initSerialPort(serialPortName);

            auto gpsDataStore = new GPSDataStore();
            auto updater = new phd::devices::gps::GPSDataUpdater(gpsDataStore, serialPort);

            cvConfig = loadCVArgs(config_folder + "/config.json");
            runObservationMode(poison_pill, gpsDataStore, phdConfig, cvConfig, serverConfig);

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