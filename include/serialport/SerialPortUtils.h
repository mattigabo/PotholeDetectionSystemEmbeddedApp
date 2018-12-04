//
// Created by Matteo Gabellini on 03/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SERIALPORTUTILS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SERIALPORTUTILS_H

#include <list>
#include <string>
using namespace std;
namespace phd::devices::serialport {
    class SerialPortUtils {
    public:
        static list<string> listAvaiablePorts();

        static void checkForException(int operationResultCode, string operationName);
    };
}
#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_SERIALPORTUTILS_H
