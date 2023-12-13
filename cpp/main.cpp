#include <iostream>

// #include <boost-src/libs/beast/include/boost/beast.hpp>
#include <boost/beast.hpp>
#include <thread>
#include <fstream>
#include <nlohmann/json.hpp>
#include "CallOperator.h"
#include "Server.h"
//#include "servUtils.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

using json = nlohmann::json;

int main(int argc, char *argv[]) {
    // Set up networking
    auto const address = net::ip::make_address(argv[1]);
    auto port = static_cast<unsigned short>(std::atoi(argv[2]));
    net::io_context ioc{1};
    tcp::acceptor acceptor{ioc, {address, port}};
    tcp::socket socket{ioc};

    // Configure program
    CDRWriter::getInstance().setPath("../../config/log.txt");
    Config &config = Config::get();
    RequestQueue::Get().setMaxSize(config.queueSize);
    ShardedDistribution distribution(50, config.minCallDuration, config.maxCallDuration);

    // Create thread for each callOperator
    std::vector<std::thread> operatorThreads;
    for (std::size_t i = 0; i < config.numOperators; ++i) {
        operatorThreads.emplace_back([i, &distribution] {
            CallOperator callOperator(i, distribution);
            callOperator.processCalls();
        });
    }

    // Start listening
    http_server(acceptor, socket);

    ioc.run();

    return 0;
}