#include <iostream>
#include <fstream>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>

using namespace std;
using namespace rapidjson;

typedef struct Arguments {
    string method;
    string bayes;
    string svm;
    bool rotate;
} Args;

Args load(const string path_to_config) {

    Args args;

    ifstream json(path_to_config, fstream::in);

    IStreamWrapper wrapper(json);

    Document config;

    config.ParseStream(wrapper);

    if (json.is_open() && config.IsObject()) {

        cout << "Opened file " << path_to_config << endl;

        assert(config.HasMember("args"));
        assert(config["args"].HasMember("method"));
        assert(config["args"].HasMember("bayes"));
        assert(config["args"].HasMember("svm"));
        assert(config["args"].HasMember("rotate"));

        args.method = config["args"]["method"].GetString();
        args.bayes = config["args"]["bayes"].GetString();
        args.svm = config["args"]["svm"].GetString();
        args.rotate = config["args"]["rotate"].GetBool();

    } else {
        cerr << "Program arguments configuration is missing. Check it's existence or create a new config.json"
             << " under the ../res/config/ folder inside the program directory. " << endl;

        exit(-3);
    }

    json.close();

    return args;

}