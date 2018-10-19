//
// Created by Matteo Gabellini on 03/10/2018.
//

#include "SerialPort/SigrokSerialPortWrapper.h"

#include "SerialPort/SerialPort.h"
#include "SerialPort/SerialPortUtils.h"
#include <iostream>
#include <libserialport.h>

namespace phd::devices::serialport {

    SigrokSerialPortWrapper::SigrokSerialPortWrapper(string name) : SerialPort(name) {
        portStructurePointer = (sp_port **) malloc(sizeof(sp_port *));
        readingBuffer = (char *) malloc(READ_BUFFER_LENGTH);
        this->cleanReadingBuffer();
    }

    void SigrokSerialPortWrapper::initPort() {
        sp_return resultCode = sp_get_port_by_name(this->portName.c_str(), portStructurePointer);
        SerialPortUtils::checkForException(resultCode, "sp_get_port_by_name in initPort");
    }


    void SigrokSerialPortWrapper::openPort(OperationMode mode) {
        this->initPort();
        sp_return openingResult;
        switch (mode) {
            case READ:
                openingResult = sp_open(*(this->portStructurePointer), SP_MODE_READ);
                break;
            case WRITE:
                openingResult = sp_open(*(this->portStructurePointer), SP_MODE_WRITE);
                break;
            case READ_AND_WRITE:
                openingResult = sp_open(*(this->portStructurePointer), SP_MODE_READ_WRITE);
                break;
        }
        SerialPortUtils::checkForException(openingResult, "sp_open in openPort");
    }

    void SigrokSerialPortWrapper::closePort() {
        sp_close(*(this->portStructurePointer));
    }

    void SigrokSerialPortWrapper::blockingRead() {
        sp_blocking_read(*(this->portStructurePointer),
                         this->readingBuffer,
                         READ_BUFFER_LENGTH,
                         READING_TIMEOUT_MILLISECONDS);
    }

    string SigrokSerialPortWrapper::extractLine() {
        char searchedChar = '\n';
        std::size_t found = internalBuffer.find(searchedChar);
        string result = "";
        if (found != std::string::npos) {
            result = internalBuffer.substr(0, found + 1);
            internalBuffer.erase(0, found + 1);
        }
        return result;
    }

    void SigrokSerialPortWrapper::cleanReadingBuffer() {
        memset(this->readingBuffer, 0, READ_BUFFER_LENGTH);
    }

    void SigrokSerialPortWrapper::fetchDataUntilNewLineIsFounded() {
        char searchedChar = '\n';
        std::size_t found = std::string::npos;
        while (found == std::string::npos) {
            this->blockingRead();
            internalBuffer += std::string(this->readingBuffer);
            found = internalBuffer.find(searchedChar);
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