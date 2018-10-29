//
// Created by Matteo Gabellini on 29/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_RASPBERRYPIUTILS_H
#define POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_RASPBERRYPIUTILS_H


#ifdef __arm__

#define __RASPBERRYPI_PLATFORM__
#include<wiringPi.h>

#endif
namespace phd::devices::raspberry::utils {
    void setupWiringPiIfNotInizialized();
}
#endif //POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_RASPBERRYPIUTILS_H
