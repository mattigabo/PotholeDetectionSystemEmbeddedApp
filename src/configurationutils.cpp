#include "configurationutils.h"

#include <fstream>
#include <functional>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <opencv2/ml.hpp>
#include <configurationutils.h>


using namespace rapidjson;
using namespace std;

namespace phd{
    namespace configurations {

        string generateErrorMessage(string subject, string path_to_config){

            return string("Program ") +
            subject +
            string(" configuration is missing.") +
            string(" Check it's existence or create a new config.json under the ") +
            path_to_config +
            string(" folder inside the program directory. ");
        }

        void loadFromJSON(const string &path_to_config, function<void(Document*)> &loadLogic, string &errorMessage) {

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

                cout << "Opened file " << path_to_config << endl;

                assert((*configRef).HasMember("cvArgs"));
                assert((*configRef)["cvArgs"].HasMember("method"));
                assert((*configRef)["cvArgs"].HasMember("bayes"));
                assert((*configRef)["cvArgs"].HasMember("svm"));
                assert((*configRef)["cvArgs"].HasMember("rotate"));

                args.method = (*configRef)["cvArgs"]["method"].GetString();
                args.bayes = (*configRef)["cvArgs"]["bayes"].GetString();
                args.svm = (*configRef)["cvArgs"]["svm"].GetString();
                args.rotate = (*configRef)["cvArgs"]["rotate"].GetBool();

                cout << endl << "Classification method: " << args.method << endl
                << "Bayes Model Location: " << args.bayes << endl
                << "SVM Model Location: " << args.svm << endl
                << "Rotate Image: " << (args.rotate ? "True" : "False") << endl << endl;

            };

            string errorMessage = generateErrorMessage("arguments", path_to_config);

            loadFromJSON(path_to_config, loadLogic, errorMessage);

            return args;

        }

        string loadSerialPortFromConfig(string path_to_config) {
            string portName;

            function<void(Document*)> loadLogic = [&](Document* configRef) {

                cout << "Loading Serial port configuration... Opened file " << path_to_config << endl;

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

                cout << "Opened file " << path_to_config << endl;

                assert((*configRef).HasMember("server"));
                assert((*configRef)["server"].HasMember("protocol"));
                assert((*configRef)["server"].HasMember("hostname"));
                assert((*configRef)["server"].HasMember("port"));
                assert((*configRef)["server"].HasMember("api"));

                serverConfig.protocol = (*configRef)["server"]["protocol"].GetString();
                serverConfig.hostname = (*configRef)["server"]["hostname"].GetString();
                serverConfig.port = (*configRef)["server"]["port"].GetInt();
                serverConfig.api = (*configRef)["server"]["api"].GetString();

                cout << endl << "Protocol: " << serverConfig.protocol << endl
                    << "Hostname/IP: " << serverConfig.hostname << endl
                    << "Port: " << serverConfig.port << endl
                    << "API endpoint: " << serverConfig.api << endl << endl;

            };

            string errorMessage = generateErrorMessage("Server", path_to_config);

            loadFromJSON(path_to_config, loadLogic, errorMessage);
            return serverConfig;
        }

        MLOptions<SVMParams> loadSVMOptions(const string &path_to_config) {

            MLOptions<SVMParams> config;
            SVMParams params;

            function<void (Document*)> parser = [&](Document* json) {

                cout << "Opened file " << path_to_config << endl;

                assert((*json).HasMember("svm"));

                // training and testing sets and algorithm model
                assert((*json)["svm"].HasMember("train-set"));
                assert((*json)["svm"]["train-set"].IsString());
                assert((*json)["svm"].HasMember("test-set"));
                assert((*json)["svm"]["test-set"].IsString());
                assert((*json)["svm"].HasMember("model"));
                assert((*json)["svm"]["model"].IsString());

                // normalization options
                assert((*json)["svm"].HasMember("norm-method"));
                assert((*json)["svm"]["norm-method"].IsString());
                assert((*json)["svm"].HasMember("norm-range"));
                assert((*json)["svm"]["norm-range"].IsArray());

                // svm-specific
                assert((*json)["svm"].HasMember("type"));
                assert((*json)["svm"]["type"].IsString());
                assert((*json)["svm"].HasMember("kernel"));
                assert((*json)["svm"]["kernel"].IsString());
                assert((*json)["svm"].HasMember("max-iter"));
                assert((*json)["svm"]["max-iter"].IsInt());
                assert((*json)["svm"].HasMember("epsilon"));
                assert((*json)["svm"]["epsilon"].IsDouble());

                // svm-specific -cross-train only
                assert((*json)["svm"].HasMember("k-fold"));
                assert((*json)["svm"]["k-fold"].IsInt());
                assert((*json)["svm"].HasMember("balanced-folding"));
                assert((*json)["svm"]["balanced-folding"].IsBool());

                // svm-specific -train only
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
                        std::cerr << "Unknown SVM type " << type << ". Using default type C_SVC." << std::endl;
                        return cv::ml::SVM::Types::C_SVC;
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
                        std::cerr << "Unknown kernel " << kernel << ". Using default kernel RBF." << std::endl;
                        return cv::ml::SVM::KernelTypes::RBF;
                    }
                };

                auto norm_parser = [](string norm_method) -> cv::NormTypes {

                    string stub = string(norm_method.data());

                    std::transform(stub.begin(), stub.end(), stub.begin(), ::toupper);
                    std::transform(stub.begin(), stub.end(), stub.begin(), [](char c) {
                        return c == '-' || c == ' ' ? '_' : c;
                    });

                    if (stub.find("NORM_") != string::npos) {
                        stub.erase(0, string("NORM_").length());
                    }

                    if (stub == "MIN_MAX" || stub == "MINMAX") {
                        return cv::NormTypes::NORM_MINMAX;
                    } else if (stub == "L1"){
                        return cv::NormTypes::NORM_L1;
                    } else if (stub == "L2"){
                        return cv::NormTypes::NORM_L2;
                    } else if (stub == "HAMMING"){
                        return cv::NormTypes::NORM_HAMMING;
                    } else if (stub == "HAMMING2"){
                        return cv::NormTypes::NORM_HAMMING2;
                    } else if (stub == "INF"){
                        return cv::NormTypes::NORM_INF;
                    } else if (stub == "RELATIVE"){
                        return cv::NormTypes::NORM_RELATIVE;
                    } else {
                        std::cerr << "Unknown normalization method " << norm_method << ". Using default method MINMAX." << std::endl;
                        return cv::NormTypes::NORM_MINMAX;
                    }
                };

                config.train_set = (*json)["svm"]["train-set"].GetString();
                config.test_set = (*json)["svm"]["test-set"].GetString();
                config.model = (*json)["svm"]["model"].GetString();
                config.norm_method = norm_parser((*json)["svm"]["norm-method"].GetString());

                if ((*json)["svm"]["norm-range"].GetArray().Size() >= 2 &&
                    (*json)["svm"]["norm-range"].GetArray()[0].IsNumber() &&
                    (*json)["svm"]["norm-range"].GetArray()[1].IsNumber()) {

                    float first = (*json)["svm"]["norm-range"].GetArray()[0].GetFloat();
                    float second = (*json)["svm"]["norm-range"].GetArray()[1].GetFloat();
                    config.norm_range = std::pair<float, float>(first, second);
                } else {
                    std::cerr << "Undefined range of normalization. Using default range [0.1, 0.9]." << std::endl;
                    config.norm_range = std::pair<float, float>(0.1, 0.9);
                }

                params.k_fold = (*json)["svm"]["k-fold"].GetInt();
                params.max_iter = (*json)["svm"]["max-iter"].GetInt();
                params.epsilon = (*json)["svm"]["epsilon"].GetDouble();
                params.C = (*json)["svm"]["C"].GetDouble();
                params.gamma = (*json)["svm"]["gamma"].GetDouble();
                params.type = type_parser((*json)["svm"]["type"].GetString());
                params.kernel = kernel_parser((*json)["svm"]["kernel"].GetString());
                params.balanced_folding = (*json)["svm"]["balanced-folding"].GetBool();
                config.params = std::pair<string, SVMParams>("svm", params);

                std::cout << std::endl
                << "train-set:" << config.train_set <<  std::endl
                << "test-set:" << config.test_set <<  std::endl
                << "model:" << config.model <<  std::endl
                << "norm:" << (*json)["svm"]["norm-method"].GetString() << " | def: MIN-MAX" << std::endl
                << "range:[" << config.norm_range.first << "," << config.norm_range.second << "]" <<  std::endl
                << "type:" << (*json)["svm"]["type"].GetString() << " | def: C_SVC" << std::endl
                << "kernel:" << (*json)["svm"]["kernel"].GetString() << " | def: RBF" <<  std::endl
                << "k-fold:" << params.k_fold <<  std::endl
                << "balanced-folding:" << (params.balanced_folding ? "true" : "false") <<  std::endl
                << "max-iter:" << params.max_iter <<  std::endl
                << "epsilon:" << params.epsilon <<  std::endl
                << "C:" << params.C <<  std::endl
                << "gamma:" << params.gamma <<  std::endl << std::endl;
            };

            string errorMessage = generateErrorMessage("SVM", path_to_config);

            loadFromJSON(path_to_config, parser, errorMessage);

            return config;
        }

        CommandLineArgs parseCommandLine(int argc, char *argv[]) {

            std::cout << "Parsing cmd-line arguments... ";

            auto mode = std::string(argv[1]);

            bool withoutRx = false;
            bool useCamera = false;
            bool saveAxelValues = false;
            string axelOutputLocation = "../res/axel.output";

            auto evaluator = [&withoutRx, &useCamera, &saveAxelValues, &axelOutputLocation](int argc, int idx, char* argv[]) {
                if (std::strcmp(argv[idx], "-withoutRx") == 0) {
                    withoutRx = true;
                } else if (std::strcmp(argv[idx], "-camera") == 0) {
                    useCamera = true;
                } else if (std::strcmp(argv[idx], "-save-axel") == 0) {
                    saveAxelValues = true;
                    if (argc > idx + 1 && std::string(argv[idx + 1]).at(0) != '-') {
                        axelOutputLocation = std::string(argv[idx + 1]);
                    }
                }
            };

            if (argc > 2) {
                for (int i = 2; i < argc; ++i) {
                    std::cout << argv[i] << "|";
                    evaluator(argc, i, argv);
                }
            }

            std::cout << "." << endl;

            return {mode, withoutRx, useCamera, saveAxelValues, axelOutputLocation};
        }

    }};