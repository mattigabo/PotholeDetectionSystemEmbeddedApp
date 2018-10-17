//
// Created by Xander on 27/9/2018.
//

#include "networking.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#include <iostream>
#include <fstream>
#include <stdio.h>

using namespace rapidjson;
using namespace std;

namespace phd::devices::networking {

    string getURL(const ServerConfig config) {
        return getURL(config.protocol, config.hostname, config.port, config.api);
    }

    string getURL(const string protocol, const string hostname, const int port, const string api) {
        return string().append(protocol).append("://")
                .append(hostname).append(":").append(to_string(port))
                .append(api);
    }

    ServerConfig loadServerConfig(string path_to_config) {

        ServerConfig serverConfig;

        ifstream json(path_to_config, fstream::in);

        IStreamWrapper wrapper(json);

        Document config;

        config.ParseStream(wrapper);

        if (json.is_open() && config.IsObject()) {

            cout << "Opened file " << path_to_config << endl;

            assert(config.HasMember("server"));
            assert(config["server"].HasMember("protocol"));
            assert(config["server"].HasMember("hostname"));
            assert(config["server"].HasMember("port"));
            assert(config["server"].HasMember("api"));

            serverConfig.protocol = config["server"]["protocol"].GetString();
            serverConfig.hostname = config["server"]["hostname"].GetString();
            serverConfig.port = config["server"]["port"].GetInt();
            serverConfig.api = config["server"]["api"].GetString();

        } else {
            cerr << "Program Server configuration is missing. Check it's existence or create a new config.json"
                 << " under the ../res/config/ folder inside the program directory. " << endl;

            exit(-3);
        }

        json.close();

        return serverConfig;
    }

    namespace HTTP {

        CURLcode init() {
            return curl_global_init(CURL_GLOBAL_ALL);
        }

        void close() {
            curl_global_cleanup();
        }

        CURLcode POST(
                std::string url,
                std::vector<std::pair<std::string, std::string>> headers,
                std::string &payload
            ) {


            CURL *curl  = curl_easy_init();
            curl_slist *_headers = NULL;

            if (curl == NULL) {
                return CURLE_FAILED_INIT;
            }

            cout << url << endl;

            for (auto p : headers) {
                auto h = std::string().append(p.first).append(": ").append(p.second);
                _headers = curl_slist_append(_headers, h.data());
            }

            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.x");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.data());

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(_headers);
            curl_easy_cleanup(curl);

            return res;
        }

        CURLcode GET (
                std::string url,
                std::vector<std::pair<std::string, std::string>> headers,
                std::vector<std::pair<std::string, std::string>> params
            ) {

            CURL *curl = curl_easy_init();;
            curl_slist *_headers = NULL;

            if (curl == NULL) {
                return CURLE_FAILED_INIT;
            }

            for (auto p : headers) {
                auto h = std::string().append(p.first).append(": ").append(p.second);
                _headers = curl_slist_append(_headers, h.data());
            }

            curl_slist_append(_headers, "Accept: application/x-www-form-urlencoded");
            curl_slist_append(_headers, "Content Type: application/x-www-form-urlencoded");
            curl_slist_append(_headers, "charset: utf-8");

            curl_easy_setopt(curl, CURLOPT_URL, url.data());
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.x");

            CURLcode res = curl_easy_perform(curl);
            curl_slist_free_all(_headers);
            curl_easy_cleanup(curl);

            return res;
        }

    }

}