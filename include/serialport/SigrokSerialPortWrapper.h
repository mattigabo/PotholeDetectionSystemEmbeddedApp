//
// Created by Matteo Gabellini on 03/10/2018.
//

#ifndef POTHOLEDETECTIONEMBEDDEDAPP_SIGROKSERIALPORTWRAPPER_H
#define POTHOLEDETECTIONEMBEDDEDAPP_SIGROKSERIALPORTWRAPPER_H


#include "SerialPort.h"
#include <libserialport.h>
#include <string>
#include <chrono>

using namespace std;

namespace phd::devices::serialport {
    class SigrokSerialPortWrapper : public SerialPort {
    public:
        static const size_t READ_BUFFER_LENGTH = 10000;
        static const int READING_TIMEOUT_MILLISECONDS = 1000;
        SigrokSerialPortWrapper(string name);
        void initPort();
        void openPort(OperationMode mode);
        void closePort();
        string readLine();

    private:
        void blockingRead();
        void fetchDataUntilNewLineIsFounded();
        void cleanReadingBuffer();
        string extractLine();
        sp_port **portStructurePointer;
        char *readingBuffer;
        string internalBuffer;
    };
}

#endif //POTHOLEDETECTIONEMBEDDEDAPP_SIGROKSERIALPORTWRAPPER_H
