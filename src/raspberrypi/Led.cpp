//
// Created by Matteo Gabellini on 29/10/2018.
//

#include "raspberrypi/Led.h"

#include <raspberrypi/raspberrypiutils.h>

#ifndef __RASPBERRYPI_PLATFORM__
#include <iostream>

using namespace std;
#endif

using namespace phd::raspberry::utils;

namespace phd::devices::raspberry::led {
    Led::Led(int pin) {
        this->pin = pin;
        this->switched_on = false;

#ifdef __RASPBERRYPI_PLATFORM__
        setupWiringPiIfNotInitialized();
        pinMode (pin, OUTPUT);
#endif
    }

    void Led::switchOn(){
#ifdef __RASPBERRYPI_PLATFORM__
        digitalWrite (pin, HIGH);
#else
        cout<< "LED pin: " << pin << " switched on"<< endl;
#endif
        this->switched_on = true;
    }

    void Led::switchOff() {
#ifdef __RASPBERRYPI_PLATFORM__
        digitalWrite (pin,  LOW);
#else
        cout<< "LED pin: " << pin << " switched off"<< endl;
#endif
        this->switched_on = false;
    }

    bool Led::isSwitchedOn() {
        return this->switched_on;
    }
}