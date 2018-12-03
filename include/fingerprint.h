//
// Created by Xander on 03/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_FINGERPRINT_H
#define POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_FINGERPRINT_H

#include <string>

namespace fingerprint {

    std::string getUID ();

    bool validateUID(std::string uid);
}

#endif //POTHOLEDETECTIONSYSTEMEMBEDDEDAPP_FINGERPRINT_H
