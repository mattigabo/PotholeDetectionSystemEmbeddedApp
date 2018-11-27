#include "ConfigurationUtils.h"

#include <fstream>
#include <functional>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

using namespace rapidjson;

namespace phd::configurations {

    string generateErrorMessage(string subject, string path_to_config){
        return string("Program ") +
                subject +
                string(" configuration is missing.") +
                string(" Check it's existence or create a new config.json under the ") +
                path_to_config +
                string("folder inside the program directory. ");
    }

    void loadFromJSON(const string path_to_config, function<void(Document*)> loadLogic, string errorMessage) {
        ifstream json(path_to_config, fstream::in);

        IStreamWrapper wrapper(json);

        Document config;
        config.ParseStream(wrapper);

        if (json.is_open() && config.IsObject()) {

            loadLogic(&config);
        } else {
            cerr << errorMessage << endl;

            exit(-3);
        }

        json.close();
    }

    CvArgs loadCvConfig(const string path_to_config) {

        CvArgs args;

        function<void(Document*)> loadLogic = [&](Document* configRef) {

            assert((*configRef).HasMember("cvArgs"));
            assert((*configRef)["cvArgs"].HasMember("method"));
            assert((*configRef)["cvArgs"].HasMember("bayes"));
            assert((*configRef)["cvArgs"].HasMember("svm"));
            assert((*configRef)["cvArgs"].HasMember("rotate"));

            args.method = (*configRef)["cvArgs"]["method"].GetString();
            args.bayes = (*configRef)["cvArgs"]["bayes"].GetString();
            args.svm = (*configRef)["cvArgs"]["svm"].GetString();
            args.rotate = (*configRef)["cvArgs"]["rotate"].GetBool();

        };

        string errorMessage = generateErrorMessage("arguments", path_to_config);

        loadFromJSON(path_to_config, loadLogic, errorMessage);

        return args;

    }

    string loadSerialPortFromConfig(string path_to_config) {
        string portName;

        function<void(Document*)> loadLogic = [&](Document* configRef) {
            cout << "Serial port configuration Loading... Opened file " << path_to_config << endl;

            assert((*configRef).HasMember("gps"));
            assert((*configRef)["gps"].HasMember("serialPort"));

            portName = (*configRef)["gps"]["serialPort"].GetString();
        };

        string errorMessage = generateErrorMessage("serialPort", path_to_config);

        loadFromJSON(path_to_config, loadLogic, errorMessage);
        return portName;
    }


    ServerConfig loadServerConfig(string path_to_config) {

        ServerConfig serverConfig;

        function<void(Document*)> loadLogic = [&](Document* configRef) {
            cout << "Server configuration Loading... Opened file " << path_to_config << endl;

            assert((*configRef).HasMember("server"));
            assert((*configRef)["server"].HasMember("protocol"));
            assert((*configRef)["server"].HasMember("hostname"));
            assert((*configRef)["server"].HasMember("port"));
            assert((*configRef)["server"].HasMember("api"));

            serverConfig.protocol = (*configRef)["server"]["protocol"].GetString();
            serverConfig.hostname = (*configRef)["server"]["hostname"].GetString();
            serverConfig.port = (*configRef)["server"]["port"].GetInt();
            serverConfig.api = (*configRef)["server"]["api"].GetString();

        };

        string errorMessage = generateErrorMessage("Server", path_to_config);

        loadFromJSON(path_to_config, loadLogic, errorMessage);
        return serverConfig;
    }
};