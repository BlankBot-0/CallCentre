//
// Created by vinilco on 11/23/23.
//

#include <random>
#include "Server.h"

http_connection::http_connection(tcp::socket socket, int deadlineDuration)
        : socket_(std::move(socket)), deadlineDuration_(deadlineDuration) {}

void http_connection::setDeadline(int deadlineDuration) {
    deadlineDuration_ = deadlineDuration;
}

std::size_t http_connection::requestCount() {
    std::lock_guard<std::mutex> lock(countMutex_);
    static std::size_t count = 0;
    return ++count;
}

void http_connection::start() {
    read_request();
    check_deadline();
}

void http_connection::read_request() {
    BOOST_LOG_TRIVIAL(trace) << "Reading request";
    auto self = shared_from_this();

    http::async_read(
            socket_,
            buffer_,
            request_,
            [self](beast::error_code ec,
                   std::size_t bytes_transferred) {
                boost::ignore_unused(bytes_transferred);
                if (!ec)
                    self->process_request();
            });
}

void http_connection::process_request() {
    response_.set(http::field::content_type, "text/plain");
    response_.version(request_.version());
    response_.keep_alive(false);

    if (request_.method() == http::verb::post) {
        clientNumber_ = request_.body();

        if (RequestQueue::Get().contains(clientNumber_)) {
            // Client's call is already pending
            BOOST_LOG_TRIVIAL(trace) << "Refusal to handle request, already pending clientNumber=" << clientNumber_;
            response_.result(http::status::too_many_requests);
            beast::ostream(response_.body()) << "already in queue";

        } else if (RequestQueue::Get().isFull()) {
            // Queue is overloaded
            std::size_t callID = requestCount();
            std::unique_ptr<Call> incomingCall = std::make_unique<Call>(
                    callID,
                    std::chrono::system_clock::now(),
                    clientNumber_
            );

            incomingCall->setCallStatus(OVERLOAD);
            response_.result(http::status::service_unavailable);
            beast::ostream(response_.body()) << "queue is full";

            BOOST_LOG_TRIVIAL(trace) << "Writing to CDR about not handling request due to queue overload";
            CDRWriter::getInstance().writeToCDR(incomingCall->getReport());

        } else {
            // Push client's call to queue
            std::size_t callID = requestCount();
            BOOST_LOG_TRIVIAL(trace) << "Enqueueing call " << callID;
            std::unique_ptr<Call> incomingCall = std::make_unique<Call>(
                    callID,
                    std::chrono::system_clock::now(),
                    clientNumber_
            );

            RequestQueue::Get().push(std::move(incomingCall));
            response_.result(http::status::ok);
            beast::ostream(response_.body()) << "callID: " << callID;
        }

    } else {
        // We return responses indicating an error if
        // we do not recognize the request method.
        BOOST_LOG_TRIVIAL(trace) << "Request recognized as unsupported";
        response_.result(http::status::bad_request);
        beast::ostream(response_.body())
                << "Invalid request-method '"
                << std::string(request_.method_string())
                << "'";
    }
    write_response();
}

void http_connection::write_response() {
    BOOST_LOG_TRIVIAL(trace) << "Writing http response";
    auto self = shared_from_this();

    response_.content_length(response_.body().size());

    http::async_write(
            socket_,
            response_,
            [self](beast::error_code ec, std::size_t) {
                // we want to be able to receive client's request
                // about them cancelling the call and thus
                // shutdown the socket only upon this event or
                // being done processing request
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
            });
}

void http_connection::check_deadline() {
    BOOST_LOG_TRIVIAL(trace) << "Connection expiration check";
    auto self = shared_from_this();

    deadline_.async_wait(
            [self](beast::error_code ec) {
                if (!ec) {
                    // Delete client's request from queue and update CDR
                    BOOST_LOG_TRIVIAL(debug) << "Deleting call due to connection timeout";
                    std::unique_ptr<Call> call = RequestQueue::Get().eraseRequest(self->clientNumber_);
                    call->setCallStatus(CallStatus::TIMEOUT);
                    BOOST_LOG_TRIVIAL(trace) << "Writing expired call information to CDR";
                    CDRWriter::getInstance().writeToCDR(call->getReport());

                    // Close socket to cancel any outstanding operation.
                    self->socket_.close(ec);
                }
            });
}

std::mt19937 rng{};

void http_server(tcp::acceptor &acceptor, tcp::socket &socket) {
    Config &config = Config::get();
    acceptor.async_accept(socket,
                          [&](beast::error_code ec) {
                              std::uniform_int_distribution<> deadlineDist{config.minDeadlineDuration,
                                                                           config.maxDeadlineDuration};
                              //BOOST_LOG_TRIVIAL(trace) << "Generating connection expiration period";
                              //BOOST_LOG_TRIVIAL(trace) << "RNG for connection expiration period created";
                              //BOOST_LOG_TRIVIAL(trace) << "Distribution of connection expiration period created";
                              int deadlineDuration = deadlineDist(rng);
                              //BOOST_LOG_TRIVIAL(trace) << "Connection expiration period set";
                              if (!ec) {
                                  BOOST_LOG_TRIVIAL(debug) << "Setting up new connection";
                                  std::make_shared<http_connection>(std::move(socket), deadlineDuration)->start();
                              } else {
                                  BOOST_LOG_TRIVIAL(error) << "Error occured when accepting a connection: " << ec;
                              }
                              BOOST_LOG_TRIVIAL(trace) << "Listening for new requests";
                              http_server(acceptor, socket);
                          });
}