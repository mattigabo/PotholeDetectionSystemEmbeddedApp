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
        /**
         * Create a GPSDataUpdater
         * @param storage an initialized data storage where the updater will save the parsed data
         * @param dataSource an opened serial port where the the updater read data
         * */
        GPSDataUpdater(GPSDataStore *storage, SerialPort *dataSource);

        void kill();

        void join();

    protected:
        /**
        * Create a GPSDataUpdater
        * @param storage an initialized data storage where the updater will save the gps data
        * */
        explicit GPSDataUpdater(GPSDataStore *storage);

        std::thread soul;
        bool should_live;
        std::function<void(void)> behaviour;
        GPSDataStore *storage;
        SerialPort *data_source;

        void Live();
    };

    class SimulatedGPSDataUpdater : public GPSDataUpdater{
    public:
        explicit SimulatedGPSDataUpdater(GPSDataStore *storage);
    };
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_GPSDATAUPDATER_H
