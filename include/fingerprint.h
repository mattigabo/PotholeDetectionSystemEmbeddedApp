//
// Created by Xander on 03/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_FINGERPRINT_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_FINGERPRINT_H

#include <string>

namespace fingerprint {

    std::string extractSerialNumber(std::string readString);

    std::string getUID ();

    bool validateUID(std::string uid);
}

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_FINGERPRINT_H
