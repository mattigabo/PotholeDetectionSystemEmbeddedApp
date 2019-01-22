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
#include <thread>
#include <mutex>
#include <deque>
#include <vector>
#include <stdio.h>

using namespace rapidjson;
using namespace std;

namespace phd{
    namespace devices {
        namespace networking {

            const int TIMEOUT_SECONDS = 5;

            string getURL(const phd::configurations::ServerConfig config) {
                return getURL(config.protocol, config.hostname, config.port, config.api);
            }

            string getURL(const string protocol, const string hostname, const int port, const string api) {
                return string().append(protocol).append("://")
                .append(hostname).append(":").append(to_string(port))
                .append(api);
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
                        std::string payload) {

                    CURL *curl = curl_easy_init();
                    curl_slist *_headers = NULL;

                    if (curl == NULL) {
                        return CURLE_FAILED_INIT;
                    }

//                    cout << url << endl;

                    for (auto p : headers) {
                        auto h = std::string().append(p.first).append(": ").append(p.second);
                        _headers = curl_slist_append(_headers, h.data());
                    }

                    curl_easy_setopt(curl, CURLOPT_URL, url.data());
                    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
                    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
                    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.x");
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.data());
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);

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

                    curl_easy_setopt(curl, CURLOPT_URL, url.data());
                    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
                    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, _headers);
                    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.x");
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, TIMEOUT_SECONDS);

                    CURLcode res = curl_easy_perform(curl);

                    // ToDo

                    curl_slist_free_all(_headers);
                    curl_easy_cleanup(curl);

                    return res;
                }

                namespace async {

                    template<typename T>
                    class ThreadSafeBuffer {
                    private:
                        std::deque<T> buffer;
                        std::mutex internal_mutex;

                    public:
                        ThreadSafeBuffer (){
                            buffer = std::deque<T>();
                        }

                        void push(const T item){

                            //Acquire the mutex and automatic release when exit from this scope
                            std::lock_guard <std::mutex> lock(internal_mutex);

                            buffer.push_back(item);
                        }

                        T pop(){
                            //Acquire the mutex and automatic release when exit from this scope
                            std::lock_guard <std::mutex> lock(internal_mutex);
                            if (!this->isEmpty()) {
                                T item = buffer.at(0);
                                buffer.pop_front();
                                return item;
                            } else {
                                return nullptr;
                            }
                        }

                        bool isEmpty() {
                            return buffer.size() == 0;
                        }
                    };


                    class HTTPRequestHandler {
                    private:

                        std::thread executor;
                        ThreadSafeBuffer<std::function<void(void)>> tsb;
                        bool is_alive;

                    public:

                        HTTPRequestHandler() {
                            this->is_alive = true;

                            this->executor = std::thread([this]() {
                                unsigned long long exec_id = 0;
                                while(this->is_alive || !this->tsb.isEmpty()) {
                                    auto handle = this->tsb.pop();
                                    if (handle != nullptr) {

                                        cout << "[Async HTTP Request Handler][OpID:" << exec_id << "]"
                                        << "Received new handle to execute." << endl;

                                        handle();

                                        exec_id++;
                                    }
                                }
                            });
                        }

                        void submit(std::function<void(void)> handle) {
                            this->tsb.push(handle);
                        }

                        void kill() {

                            this->is_alive = false;
                            this->executor.join();
                        }
                    };

                    static HTTPRequestHandler* httpRequestHandler;

                    CURLcode init() {
                        httpRequestHandler = new HTTPRequestHandler();
                        return curl_global_init(CURL_GLOBAL_ALL);
                    }

                    void close() {
                        httpRequestHandler->kill();
                        curl_global_cleanup();
                    }

                    void POST(std::string url,
                              std::vector<std::pair<std::string, std::string>> headers,
                              std::string payload,
                              std::function<void(CURLcode)> callback,
                              phd::devices::raspberry::led::Led *led) {

                        httpRequestHandler->submit([url, headers, payload, callback, led]() {
                            if (led != nullptr && !led->isSwitchedOn()) {
                                led->switchOn();
                            }
                            callback(HTTP::POST(url, headers, payload));
                        });

                    }

                }

            }


        }
    }}