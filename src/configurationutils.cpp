#include "configurationutils.h"

#include <fstream>
#include <functional>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <opencv2/ml.hpp>

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

    CVArgs loadCVArgs(const string path_to_config) {

        CVArgs args;

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

    CrossValidationArgs<SVMParams> loadSVMCrossValidationArgs(const string &path_to_config) {

        CrossValidationArgs<SVMParams> config;
        SVMParams params;

        function<void (Document*)> parser = [&](Document* json) {
            cout << "Loading Support Vector Machine Configuration... Opened file " << path_to_config << endl;

            assert((*json).HasMember("svm"));
            assert((*json)["svm"].HasMember("train-set"));
            assert((*json)["svm"]["train-set"].IsString());
            assert((*json)["svm"].HasMember("test-set"));
            assert((*json)["svm"]["test-set"].IsString());
            assert((*json)["svm"].HasMember("model"));
            assert((*json)["svm"]["model"].IsString());
            assert((*json)["svm"].HasMember("type"));
            assert((*json)["svm"]["type"].IsString());
            assert((*json)["svm"].HasMember("kernel"));
            assert((*json)["svm"]["kernel"].IsString());
            assert((*json)["svm"].HasMember("k-fold"));
            assert((*json)["svm"]["k-fold"].IsInt());
            assert((*json)["svm"].HasMember("max-iter"));
            assert((*json)["svm"]["max-iter"].IsInt());
            assert((*json)["svm"].HasMember("epsilon"));
            assert((*json)["svm"]["epsilon"].IsDouble());
            assert((*json)["svm"].HasMember("C"));
            assert((*json)["svm"]["C"].IsDouble());
            assert((*json)["svm"].HasMember("gamma"));
            assert((*json)["svm"]["gamma"].IsDouble());

            auto type_parser = [](const string type) -> cv::ml::SVM::Types {

                string stub = string(type.data());
                std::transform(stub.begin(), stub.end(), stub.begin(), ::toupper);
                std::transform(stub.begin(), stub.end(), stub.begin(), [](char c) {
                    return c == '-' || c == ' ' ? '_' : c;
                });

                if (stub == "C_SVC" || stub == "CSVC") {
                    return cv::ml::SVM::Types::C_SVC;
                } else if (stub == "NU_SVC" || stub == "NUSVC"){
                    return cv::ml::SVM::Types::NU_SVC;
                } else if (stub == "EPS_SVR" || stub == "EPSSVR"){
                    return cv::ml::SVM::Types::EPS_SVR;
                } else if (stub == "NU_SVR" || stub == "NUSVR"){
                    return cv::ml::SVM::Types::NU_SVR;
                } else if (stub == "ONE_CLASS" || stub == "ONECLASS"){
                    return cv::ml::SVM::Types::ONE_CLASS;
                } else {
                    throw cv::Exception(
                            -666,"Unknown SVM Type " + type,
                            "type_parser = [](string type) -> cv::ml::SVM::Types",
                            "configurationutils.cpp", 164
                    );
                }
            };

            auto kernel_parser = [](string kernel) -> cv::ml::SVM::KernelTypes {
                string stub = string(kernel.data());
                std::transform(stub.begin(), stub.end(), stub.begin(), ::toupper);
                if (stub == "RBF") {
                    return cv::ml::SVM::KernelTypes::RBF;
                } else if (stub == "LINEAR"){
                    return cv::ml::SVM::KernelTypes::LINEAR;
                } else if (stub == "POLY"){
                    return cv::ml::SVM::KernelTypes::POLY;
                } else if (stub == "SIGMOID"){
                    return cv::ml::SVM::KernelTypes::SIGMOID;
                } else if (stub == "CHI2"){
                    return cv::ml::SVM::KernelTypes::CHI2;
                } else if (stub == "INTER"){
                    return cv::ml::SVM::KernelTypes::INTER;
                } else {
                    throw cv::Exception(
                            -666,"Unknown Kernel Type " + kernel,
                            "kernel_parser = [](string kernel) -> cv::ml::SVM::KernelTypes",
                            "configurationutils.cpp", 186
                    );
                }
            };

            config.trainset = (*json)["svm"]["train-set"].GetString();
            config.testset = (*json)["svm"]["test-set"].GetString();
            config.model = (*json)["svm"]["model"].GetString();
            params.kfold = (*json)["svm"]["k-fold"].GetInt();
            params.max_iter = (*json)["svm"]["max-iter"].GetInt();
            params.epsilon = (*json)["svm"]["epsilon"].GetDouble();
            params.C = (*json)["svm"]["C"].GetDouble();
            params.gamma = (*json)["svm"]["gamma"].GetDouble();
            params.type = type_parser((*json)["svm"]["type"].GetString());
            params.kernel = kernel_parser((*json)["svm"]["kernel"].GetString());
            config.params = std::pair<string, SVMParams>("svm", params);
        };

        string errorMessage = generateErrorMessage("SVM", path_to_config);

        loadFromJSON(path_to_config, parser, errorMessage);

        std::cout << std::endl << "LOADED: " << std::endl
        << "train-set:" << config.trainset <<  std::endl
        << "test-set:" << config.testset <<  std::endl
        << "model:" << config.model <<  std::endl
        << "type:" << params.type <<  std::endl
        << "kernel:" << params.kernel <<  std::endl
        << "k-fold:" << params.kfold <<  std::endl
        << "max-iter:" << params.max_iter <<  std::endl
        << "epsilon:" << params.epsilon <<  std::endl
        << "C:" << params.C <<  std::endl
        << "gamma:" << params.gamma <<  std::endl << std::endl;

        return config;
    }
};