//
// Created by Matteo Gabellini on 03/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SIGROKSERIALPORTWRAPPER_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SIGROKSERIALPORTWRAPPER_H


#include "SerialPort.h"
#include <libserialport.h>
#include <string>
#include <chrono>

namespace phd {
    namespace devices {
        namespace serialport {
            class SigrokSerialPortWrapper : public SerialPort {
            public:
                static const size_t READ_BUFFER_LENGTH = 10000;
                static const int READING_TIMEOUT_MILLISECONDS = 1000;
                SigrokSerialPortWrapper(std::string name);
                ~SigrokSerialPortWrapper();
                void initPort();
                void openPort(OperationMode mode);
                void closePort();
                std::string readLine();

            private:
                void blockingRead();
                void fetchDataUntilNewLineIsFounded();
                void cleanReadingBuffer();
                std::string extractLine();
                sp_port **port_structure_pointer;
                char *reading_buffer;
                std::string internal_buffer;
            };
        }
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SIGROKSERIALPORTWRAPPER_H
