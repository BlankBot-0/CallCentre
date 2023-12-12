//
// Created by vinilco on 11/23/23.
//

#ifndef CALLCENTRE_SERVER_H
#define CALLCENTRE_SERVER_H

#include <memory>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
// #include "servUtils.h"
#include <map>
#include <shared_mutex>
#include "CDRwriter.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp;

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    explicit http_connection(tcp::socket socket, int deadlineDuration);
    void setDeadline(int deadlineDuration);
    void start();

private:
    std::mutex countMutex_;

    // client's number
    numberType clientNumber_;

    // deadline for connection processing
    int deadlineDuration_;

    // The socket for the currently connected client.
    tcp::socket socket_;

    // The buffer for performing reads.
    beast::flat_buffer buffer_{8192};

    // The request message.
    http::request<http::string_body> request_;

    // The response message.
    http::response<http::dynamic_body> response_;

    // The timer for putting a deadline on connection processing.
    boost::asio::steady_timer deadline_{
            socket_.get_executor(), std::chrono::seconds(deadlineDuration_)};

    std::size_t requestCount();

    // Asynchronously receive a complete request message.
    void read_request();

    // Determine what needs to be done with the request message.
    void process_request();

    // Construct a response message based on the program state.
    void create_response();

    // Asynchronously transmit the response message.
    void write_response();

    // Check whether we have spent enough time on this connection.
    void check_deadline();
};

// "Loop" forever accepting new connections.
void http_server(tcp::acceptor& acceptor, tcp::socket& socket, int minDeadlineDuration, int maxDeadlineDuration);

#endif //CALLCENTRE_SERVER_H