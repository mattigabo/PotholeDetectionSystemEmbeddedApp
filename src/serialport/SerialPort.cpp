//
// Created by Matteo Gabellini on 03/10/2018.
//

#include "serialport/SerialPort.h"
namespace phd::devices::serialport {

    SerialPort::SerialPort(string name) {
        port_name = name;
    }

}