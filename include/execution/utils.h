//
// Created by Xander on 04/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_UTILS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_UTILS_H

#include <gps/GPSDataStore.h>
#include <string>
#include <networking.h>

std::string toJSON(phd::devices::gps::Coordinates coordinates);

void sendDataToServer(std::string payload, phd::configurations::ServerConfig serverConfig);

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_UTILS_H
