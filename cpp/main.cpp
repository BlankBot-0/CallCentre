#include <iostream>
#include <getopt.h>

#include <boost/beast.hpp>
#include <thread>
#include <fstream>
#include <nlohmann/json.hpp>
#include "CallOperator.h"
#include "Server.h"
//#include "servUtils.h"
std::string logoption;
std::string IP = "127.0.0.1";
std::string uport = "8083";


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

using json = nlohmann::json;

void PrintHelp()
{
    std::cout <<
              "-i <IP_address>:     Set server IP address\n"
              "-p <port>:           Set port\n"
              "-l <log option>:     Set level of logging\n"
              "--help:              Show help\n";
    exit(1);
}

void ProcessArgs(int argc, char** argv)
{
    const char* const short_opts = "i:p:l";
    const option long_opts[] = {
            {"IPaddress", required_argument, nullptr, 'i'},
            {"port", required_argument, nullptr, 'p'},
            {"logfilter", optional_argument, nullptr, 'l'},
            {nullptr, no_argument, nullptr, 0}
    };

    if(argc < 5) {
        std::string logoption = "trace";
    }

    while (true)
    {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);

        if (-1 == opt)
            break;

        switch (opt)
        {
            case 'i':
                IP = std::string(argv[2]);
                std::cout << "IP set to: " << IP << '\n';
                break;

            case 'p':
                uport = std::string(argv[4]);
                std::cout << "Port set to: " << uport << '\n';
                break;

            case 'l':
                logoption = std::string(argv[6]);
                std::cout << "Log option set to: " << logoption << std::endl;
                break;

            case 'h': // -h or --help
                PrintHelp();
                exit(0);

            default:
                // PrintHelp();
                break;
        }
    }
}

void init()
{
    if (logoption == "trace") {
        boost::log::core::get()->set_filter
                (
                        boost::log::trivial::severity >= boost::log::trivial::trace
                );
    } else if (logoption == "bebug") {
        boost::log::core::get()->set_filter
                (
                        boost::log::trivial::severity >= boost::log::trivial::debug
                );
    } else if (logoption == "warning") {
        boost::log::core::get()->set_filter
                (
                        boost::log::trivial::severity >= boost::log::trivial::warning
                );
    } else {
        boost::log::core::get()->set_filter
                (
                        boost::log::trivial::severity >= boost::log::trivial::trace
                );
    }
}

int main(int argc, char *argv[]) {
    init();
    ProcessArgs(argc, argv);
    // Set up networking
    auto const address = net::ip::make_address(IP);
    auto port = static_cast<unsigned  short>(std::stoul(uport));
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