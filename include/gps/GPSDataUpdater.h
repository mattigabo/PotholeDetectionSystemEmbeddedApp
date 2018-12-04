//
// Created by Matteo Gabellini on 05/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_GPSDATAUPDATER_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_GPSDATAUPDATER_H


#include <iostream>
#include <thread>
#include <functional>
#include <gps/GPSDataStore.h>
#include <serialport/SerialPort.h>

using  namespace phd::devices::serialport;

namespace phd::devices::gps {
/**
 * This class represent an autonomous worker that continuously read and parse NMEA data from serial port
 * and update the GPSDataStore passed as  argument at the constuctor
 */
    class GPSDataUpdater {
    public:
        GPSDataUpdater(GPSDataStore *storage, SerialPort *dataSource);

        void kill();

        void join();

    private:
        std::thread soul;
        bool should_live;
        std::function<void(void)> behaviour;
        GPSDataStore *storage;
        SerialPort *data_source;

        void Live();
    };
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_GPSDATAUPDATER_H
