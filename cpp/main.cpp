#include <iostream>

// #include <boost-src/libs/beast/include/boost/beast.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <fstream>
#include <nlohmann/json.hpp>
#include "CallOperator.h"
#include "Server.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

using json = nlohmann::json;

int main(int argc, char* argv[]) {

    // Configure the service
    std::ifstream configFile("../config.json");
    json config = json::parse(configFile);
    configFile.close();

    int minCallDuration = config["minCallDuration"].get<int>();
    int maxCallDuration = config["maxCallDuration"].get<int>();
    int minDeadlineDuration = config["minDeadlineDuration"].get<int>();
    int maxDeadlineDuration = config["maxDeadlineDuration"].get<int>();
    int numOperators = config["numOperators"].get<int>();

    // Set up networking
    auto const address = net::ip::make_address(argv[1]);
    auto port = static_cast<unsigned short>(std::atoi(argv[2]));
    net::io_context ioc{1};
    tcp::acceptor acceptor{ioc, {address, port}};
    tcp::socket socket{ioc};

    // Instantiate singletons
    RequestQueue& requestQueue = RequestQueue::Get();
    CDRWriter::getInstance().setPath("../../config/log.txt");

    // Create thread for each callOperator
    std::vector<std::thread> operatorThreads;
    for (std::size_t i = 0; i < numOperators; ++i) {
        operatorThreads.emplace_back([i, minCallDuration, maxCallDuration] {
            CallOperator callOperator(i, minCallDuration, maxCallDuration);
            callOperator.processCalls();
        });
    }

    // Start listening
    http_server(acceptor, socket, minDeadlineDuration, maxDeadlineDuration);

    ioc.run();

    return 0;
}