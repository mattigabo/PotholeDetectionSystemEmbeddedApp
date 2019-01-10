//
// Created by Xander on 04/12/2018.
//

#ifndef POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_UTILS_H
#define POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_UTILS_H

#include <gps/GPSDataStore.h>
#include <string>
#include <networking.h>
#include <raspberrypi/led.h>

std::string toJSON(phd::devices::gps::Coordinates coordinates, std::string token);

std::string toJSON(std::string token);

void sendDataToServer(std::string payload, phd::configurations::ServerConfig serverConfig);

void sendDataToServerAsync(phd::configurations::ServerConfig serverConfig, std::string payload,
                           std::function<void(CURLcode)> callback, phd::devices::raspberry::led::Led *led);

CURLcode registerDeviceOnServer(std::string payload, phd::configurations::ServerConfig serverConfig);

template <typename T>
void print_vector(std::vector<T> v);

#endif //POTHOLEDETECTIONSYSTEM_EMBEDDEDAPP_UTILS_H
