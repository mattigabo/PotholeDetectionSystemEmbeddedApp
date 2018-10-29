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
                    //printf("$GGA: fix quality: %d\n", frame.fix_quality);
                    result = Coordinates{minmea_tocoord(&frame.latitude),
                                         minmea_tocoord(&frame.longitude),
                                         minmea_tofloat(&frame.altitude)};
                }
            }
                break;

            default:
                throw string("NMEA type ignored");
                break;
        }
        //}
        return result;
    }


    void GPSDataUpdater::Live() {
        while (should_live) {
            behaviour();
        }
    }

    /**
     * @param storage an initialized data storage where the updater will save the parsed data
     * @param dataSource an opened serial port where the the updater read data
     * */
    GPSDataUpdater::GPSDataUpdater(GPSDataStore *storage, SerialPort *dataSource) {
        this->storage = storage;
        this->data_source = dataSource;

        this->behaviour = [&]() {
            string lineRead = this->data_source->readLine();
            //std::cout << "Linea letta" << lineRead << std::endl;
            try {
                Coordinates coordinates = parseNMEAData(lineRead.c_str());
                this->storage->update(coordinates);
            } catch (const string msg) {
                //cerr << msg << endl;
            }
        };

        soul = std::thread(&GPSDataUpdater::Live, this);
        should_live = true;
    }

    void GPSDataUpdater::kill() {
        should_live = false;
    }

    void GPSDataUpdater::join() {
        soul.join();
    }
}