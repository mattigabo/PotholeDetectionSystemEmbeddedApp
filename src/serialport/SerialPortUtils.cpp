//
// Created by Matteo Gabellini on 03/10/2018.
//

#include "serialport/SerialPortUtils.h"
#include <iostream>
#include <libserialport.h>

namespace phd::devices::serialport {

    list<string> SerialPortUtils::listAvaiablePorts() {
        list<string> portNames;
        auto list_ptr = (sp_port ***) malloc(sizeof(sp_port **));
        sp_list_ports(list_ptr);
        int i = 0;
        while ((*list_ptr)[i] != NULL) {
            sp_port *port = (*list_ptr)[i];
            char *portName = sp_get_port_name(port);
            portNames.push_back(portName);
            i++;
        }
        return portNames;
    }


    void SerialPortUtils::checkForException(int operationResultCode, string operationName) {
        string err;
        if (operationResultCode == SP_ERR_ARG) {
            throw "Invalid arguments were passed to the function. " + operationName;
        } else if (operationResultCode == SP_ERR_FAIL) {
            throw "A system error occurred while executing the operation: " + operationName;
        } else if (operationResultCode == SP_ERR_MEM) {
            throw "A memory allocation failed while executing the operation: " + operationName;
        } else if (operationResultCode == SP_ERR_SUPP) {
            throw "The requested operation is not supported by this system or device. Error occured in function " +
                  operationName;
        }
    }

}