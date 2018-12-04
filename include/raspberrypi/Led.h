//
// Created by Matteo Gabellini on 29/10/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_LED_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_LED_H

namespace phd::devices::raspberry::led{

class Led {
public:
    Led(int pin);

    void switchOn();

    void switchOff();

    bool isSwitchedOn();

private:
    bool switched_on;
    int pin;
};

typedef struct NotificationLeds {
    Led programInExecution;
    Led validGpsData;
    Led serverDataTransfering;
    Led cameraIsShooting;
} NotificationLeds;

}
#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_LED_H
