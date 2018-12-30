//
// Created by Matteo Gabellini on 03/10/2018.
//

#include "serialport/SigrokSerialPortWrapper.h"

#include <serialport/SerialPort.h>
#include <serialport/SerialPortUtils.h>
#include <iostream>
#include <libserialport.h>
#include <string.h>

using namespace std;

namespace phd {
    namespace devices {
        namespace serialport {

            SigrokSerialPortWrapper::SigrokSerialPortWrapper(string name) : SerialPort(name) {
                port_structure_pointer = (sp_port **) malloc(sizeof(sp_port *));
                reading_buffer = (char *) malloc(READ_BUFFER_LENGTH);
                this->cleanReadingBuffer();
            }

            SigrokSerialPortWrapper::~SigrokSerialPortWrapper() {
                this->closePort();
                delete(reading_buffer);
                delete(port_structure_pointer);
            }

            void SigrokSerialPortWrapper::initPort() {
                sp_return resultCode = sp_get_port_by_name(this->port_name.c_str(), port_structure_pointer);
                SerialPortUtils::checkForException(resultCode, "sp_get_port_by_name in initPort");
            }


            void SigrokSerialPortWrapper::openPort(OperationMode mode) {
                this->initPort();
                sp_return openingResult;
                switch (mode) {
                    case READ:
                        openingResult = sp_open(*(this->port_structure_pointer), SP_MODE_READ);
                        break;
                        case WRITE:
                            openingResult = sp_open(*(this->port_structure_pointer), SP_MODE_WRITE);
                            break;
                            case READ_AND_WRITE:
                                openingResult = sp_open(*(this->port_structure_pointer), SP_MODE_READ_WRITE);
                                break;
                }
                SerialPortUtils::checkForException(openingResult, "sp_open in openPort");
            }

            void SigrokSerialPortWrapper::closePort() {
                sp_close(*(this->port_structure_pointer));
            }

            void SigrokSerialPortWrapper::blockingRead() {
                sp_blocking_read(*(this->port_structure_pointer),
                        this->reading_buffer,
                        READ_BUFFER_LENGTH,
                        READING_TIMEOUT_MILLISECONDS);
            }

            string SigrokSerialPortWrapper::extractLine() {
                char searchedChar = '\n';
                std::size_t found = internal_buffer.find(searchedChar);
                string result = "";
                if (found != std::string::npos) {
                    result = internal_buffer.substr(0, found + 1);
                    internal_buffer.erase(0, found + 1);
                }
                return result;
            }

            void SigrokSerialPortWrapper::cleanReadingBuffer() {
                memset(this->reading_buffer, 0, READ_BUFFER_LENGTH);
            }

            void SigrokSerialPortWrapper::fetchDataUntilNewLineIsFounded() {
                char searchedChar = '\n';
                std::size_t found = std::string::npos;
                while (found == std::string::npos) {
                    this->blockingRead();
                    internal_buffer += std::string(this->reading_buffer);
                    found = internal_buffer.find(searchedChar);
                    cleanReadingBuffer();
                }
            }

            string SigrokSerialPortWrapper::readLine() {
                string result = extractLine();
                while (result == "") {
                    fetchDataUntilNewLineIsFounded();
                    result = extractLine();
                }
                return result;
            }


        }
    }}