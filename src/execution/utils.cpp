//
// Created by Xander on 04/12/2018.
//

#include "execution/utils.h"

#include <fingerprint.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <execution/utils.h>


const vector<pair<string, string>> httpHeaders({
   pair<string, string>("Accept", "application/json"),
   pair<string, string>("Content-Type","application/json"),
   pair<string, string>("charset","utf-8")
});

std::string toJSON(phd::devices::gps::Coordinates coordinates, std::string token) {
    rapidjson::Document document;
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    document.Parse("{}");
    assert(document.IsObject());

    rapidjson::Value object(rapidjson::kObjectType);

    object.AddMember("lat", rapidjson::Value(coordinates.latitude), allocator);
    object.AddMember("lng", rapidjson::Value(coordinates.longitude), allocator);

    auto uid = rapidjson::StringRef(token.data());

    document.AddMember("token", rapidjson::Value(uid), allocator);
    document.AddMember("content", object, allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}



void sendDataToServer(std::string payload, ServerConfig serverConfig){
    CURLcode res = phd::devices::networking::HTTP::POST(
            phd::devices::networking::getURL(serverConfig),
            httpHeaders,
            payload);

    cout << "HTTP Response Code:" << res << endl;
}

template <typename T>
void print_vector(const std::vector<T> v) {
    cout << "[";
    for (int i = 0; i < v.size(); ++i) {
        cout << v.at(i);
        cout << (i + 1 < v.size() ? ", " : "");
    }
    cout << "]" << endl;
}

std::string toJSON(std::string token) {

    rapidjson::Document document;
    rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

    document.Parse("{}");
    assert(document.IsObject());

    auto uid = rapidjson::StringRef(token.data());

    document.AddMember("token", rapidjson::Value(uid), allocator);

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    document.Accept(writer);

    return buffer.GetString();
}

void registerDeviceOnServer(std::string payload, phd::configurations::ServerConfig serverConfig) {

    CURLcode res = phd::devices::networking::HTTP::POST(
            phd::devices::networking::getURL(serverConfig) + "/register",
            httpHeaders,
            payload);

    cout << "HTTP Response Code:" << res << endl;

}
