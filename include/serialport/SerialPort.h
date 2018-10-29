//
// Created by Matteo Gabellini on 02/10/2018.
//

#ifndef POTHOLEDETECTIONEMBEDDEDAPP_SERIALPORT_H
#define POTHOLEDETECTIONEMBEDDEDAPP_SERIALPORT_H

#include <string>
using namespace std;
namespace phd::devices::serialport {
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
        SerialPort(string name);

        virtual void openPort(OperationMode mode) = 0;

        virtual void closePort() = 0;

        virtual string readLine() = 0;

    protected:
        string port_name;
    };
}

#endif //POTHOLEDETECTIONEMBEDDEDAPP_SERIALPORT_H
