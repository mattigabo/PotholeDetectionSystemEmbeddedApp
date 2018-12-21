//
// Created by Matteo Gabellini on 29/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_RASPBERRYPIUTILS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_RASPBERRYPIUTILS_H

/*
 * to date, unfortunately this is the only mode to distinguish the RaspberryPi architecture
 * from other typical notebook computer architectures.
 * */
#ifdef __arm__

#define __RASPBERRYPI_PLATFORM__
#include<wiringPi.h>

#endif
namespace phd {
    namespace raspberry {
        namespace utils {
            void setupWiringPiIfNotInitialized();
        }
    }
}
#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_RASPBERRYPIUTILS_H
