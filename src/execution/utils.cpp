//
// Created by Xander on 04/12/2018.
//

#include "execution/utils.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

const vector<pair<string, string>> httpHeaders({
   pair<string, string>("Accept", "application/json"),
   pair<string, string>("Content-Type","application/json"),
   pair<string, string>("charset","utf-8")
});

std::string toJSON(phd::devices::gps::Coordinates coordinates) {
    rapidjson::Document document;

    document.Parse("{}");

    assert(document.IsObject());

    document.AddMember("lat", rapidjson::Value(coordinates.latitude), document.GetAllocator());
    document.AddMember("lng", rapidjson::Value(coordinates.longitude), document.GetAllocator());

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