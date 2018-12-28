//
// Created by Matteo Gabellini on 02/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SERIALPORT_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SERIALPORT_H

#include <string>

namespace phd{
    namespace devices {
        namespace serialport {
            /**
            * Mode of operation of the serial ports
            * */
            enum OperationMode {
                /**
                * To open serial port in read only mode
                * */
                READ = 0,
                /**
                * To open serial port in write only mode
                * */
                WRITE = 1,
                /**
                * To open serial port in read only mode
                * */
                READ_AND_WRITE = 2
            };


            class SerialPort {
            public:
                SerialPort(std::string name);

                virtual void openPort(OperationMode mode) = 0;

                virtual void closePort() = 0;

                virtual std::string readLine() = 0;

            protected:
                std::string port_name;
            };
        }
    }
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SERIALPORT_H
