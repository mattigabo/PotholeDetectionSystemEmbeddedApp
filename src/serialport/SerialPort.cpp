//
// Created by Matteo Gabellini on 03/10/2018.
//

#include "serialport/SerialPort.h"


using namespace std;

namespace phd {
    namespace devices {
        namespace serialport {

            SerialPort::SerialPort(string name) {
                port_name = name;
            }
        }
    }
}