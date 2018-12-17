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
#include <accelerometer/ml.h>
#include <phdetection/io.hpp>

#include <raspberrypi/raspberrypiutils.h>

#include <execution/observables/camera.h>
#include <execution/observers/camera.h>
#include <execution/observables/gps.h>
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
string serialPortName;

string config_folder = "/res/config";
NotificationLeds notificationLeds = { Led(0), Led(1), Led(2), Led(3)};

void showHelper(void) {

    cout << "-o [== Run Observation process on the RasPi Camera]" << endl;
    cout << "-gps [== Test the gps communication]" << endl;
    cout << "-http [== Test HTTP communication]" << endl;
    cout << "-led [== Test LED]" << endl;
    cout << "-accelerometer [== Test Accelerometer]" << endl;
    cout << "-train <config-file> [ == Train and Test the SVM classifier against the given train-set(s) and test-set(s)]" << endl;
    cout << "-test <config-file> [ == Trained Classify against the given test-set]" << endl;
}

SerialPort* initSerialPort(string portName){
    SerialPort* sp = new SigrokSerialPortWrapper(portName);
    sp->openPort(READ);
    return sp;
}

void initLedStructures(){

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
    cout << endl;

    if (argc < 2) {
        showHelper();

        return 0;
    } else {

        auto mode = std::string(argv[1]);

        cout << mode << " mode: ON" << endl;

        auto poison_pill = false;

        phdConfig = loadProgramConfiguration(config_folder + "/config.json");
        serverConfig = loadServerConfig(config_folder + "/config.json");

        if(mode == "-http"){
            testHTTPCommunication(serverConfig);
        } else if(mode == "-led") {
            testLed(notificationLeds);
        } else if (mode == "-accelerometer") {
            testAccelerometerCommunication();
        } else if (mode == "-train" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);

            trainAccelerometerMlAlgorithm(svmConfig, false);
//            testAccelerometerMlAlgorithm(svmConfig);

        } else if (mode == "-cross-train" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);

            trainAccelerometerMlAlgorithm(svmConfig, true);
            testAccelerometerMlAlgorithm(svmConfig);

        } else if (mode == "-test" && argc > 2) {

            auto svmConfig = loadSVMOptions(argv[2]);
            testAccelerometerMlAlgorithm(svmConfig);

        } else if (mode == "-fp") {

            testFingerPrintCalculation();
//            testB();
//            testC();

        } else {

            cout << "Registering Device on Server..." << endl;
            registerDeviceOnServer(toJSON(fingerprint::getUID()), serverConfig);

            serialPortName = loadSerialPortFromConfig(config_folder + "/config.json");

            SerialPort *serialPort = initSerialPort(serialPortName);
            auto gpsDataStore = new GPSDataStore();
            auto updater = new phd::devices::gps::GPSDataUpdater(gpsDataStore, serialPort);

            if (mode == "-o") {
                CVArgs cvConfig = loadCVArgs(config_folder + "/config.json");
                runObservationMode(poison_pill, gpsDataStore, phdConfig, cvConfig, serverConfig);
            } else if (mode == "-gps") {
                testGPSCommunication(gpsDataStore);

                observables::gps::createGPSDataStream(gpsDataStore, observables::gps::GPS_REFRESH_PERIOD);

            } else {
                showHelper();
            }

            updater->kill();
            updater->join();
            serialPort->closePort();
            delete (updater);
            delete (serialPort);
            delete (gpsDataStore);
        }
    }

    HTTP::close();
    return 1;
}