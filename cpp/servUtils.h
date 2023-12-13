//
// Created by vinilco on 12/5/23.
//

#ifndef CALLCENTRE_SERVUTILS_H
#define CALLCENTRE_SERVUTILS_H

#include <memory>
#include <fstream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp;

using json = nlohmann::json;

struct Config {
    int minCallDuration;
    int maxCallDuration;
    int minDeadlineDuration;
    int maxDeadlineDuration;
    int numOperators;
    int queueSize;

    static Config &get() {
        static Config instance;
        return instance;
    }

private:
    Config() {
        // Configure the service
        std::ifstream configFile("../config.json");
        json config = json::parse(configFile);
        configFile.close();

        minCallDuration = config["minCallDuration"].get<int>();
        maxCallDuration = config["maxCallDuration"].get<int>();
        minDeadlineDuration = config["minDeadlineDuration"].get<int>();
        maxDeadlineDuration = config["maxDeadlineDuration"].get<int>();
        numOperators = config["numOperators"].get<int>();
        queueSize = config["queueSize"].get<int>();
    }
};

#endif //CALLCENTRE_SERVUTILS_H
