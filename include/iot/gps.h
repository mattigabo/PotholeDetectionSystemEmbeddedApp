//
// Created by Xander on 26/9/2018.
//

#ifndef POTHOLEDETECTIONOBSERVER_GPS_H
#define POTHOLEDETECTIONOBSERVER_GPS_H

namespace phd::iot::gps {

    typedef struct Coordinates {
        double lat;
        double lng;
        double el;
    } Coordinates;

    Coordinates fetch();
};

#endif //POTHOLEDETECTIONOBSERVER_GPS_H
