//
// Created by Matteo Gabellini on 05/10/2018.
//

#include "gps/GPSDataUpdater.h"
#include "minmea/minmea.h"

namespace phd::devices::gps {
    /**
     * This function parse NMEA data string. The NMEA sentence considered is
     * the GGA type (Global Positioning System Fix Data. Time, Position and fix related data for a GPS receiver)
     * */
    Coordinates parseNMEAData(const char *line) {
        Coordinates result;
        switch (minmea_sentence_id(line, false)) {

            case MINMEA_SENTENCE_GGA: {
                struct minmea_sentence_gga frame;
                if (minmea_parse_gga(&frame, line)) {
                    result = Coordinates{minmea_tocoord(&frame.latitude),
                                         minmea_tocoord(&frame.longitude),
                                         minmea_tofloat(&frame.altitude)};
                }
            }
                break;

            default:
                throw string("NMEA type ignored");
        }
        return result;
    }


    void GPSDataUpdater::Live() {
        while (should_live) {
            behaviour();
        }
    }

    GPSDataUpdater::GPSDataUpdater(GPSDataStore *storage){
        this->storage = storage;
        should_live = true;
    }

    GPSDataUpdater::GPSDataUpdater(GPSDataStore *storage, SerialPort *dataSource): GPSDataUpdater(storage) {
        this->data_source = dataSource;

        this->behaviour = [&]() {
            string lineRead = this->data_source->readLine();
            //std::cout << "Line Read" << lineRead << std::endl;
            try {
                Coordinates coordinates = parseNMEAData(lineRead.c_str());
                this->storage->update(coordinates);
            } catch (const string msg) {
                //cerr << msg << endl;
            }
        };

        soul = std::thread(&GPSDataUpdater::Live, this);
    }

    void GPSDataUpdater::kill() {
        should_live = false;
    }

    void GPSDataUpdater::join() {
        soul.join();
    }

    SimulatedGPSDataUpdater::SimulatedGPSDataUpdater(GPSDataStore *storage): GPSDataUpdater(storage) {
        this->behaviour = [&]() {
            try {
                Coordinates coordinates = {.latitude=43.9414, .longitude=12.7188, .altitude=0};
                this->storage->update(coordinates);
                std::this_thread::sleep_for(1s);
            } catch (const string msg) {
                //cerr << msg << endl;
            }
        };

        soul = std::thread(&SimulatedGPSDataUpdater::Live, this);
    }
}