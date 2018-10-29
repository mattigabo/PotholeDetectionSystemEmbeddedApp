//
// Created by Matteo Gabellini on 29/10/2018.
//
#include <raspberrypi/raspberrypiutils.h>

namespace  phd::raspberry::utils {
    bool wiringPiInitialized = false;

    void setupWiringPiIfNotInitialized(){
        if(!wiringPiInitialized){
#ifdef __RASPBERRYPI_PLATFORM__
            wiringPiSetup();
#endif
        }
    }
}