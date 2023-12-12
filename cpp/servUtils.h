//
// Created by vinilco on 12/5/23.
//

#ifndef CALLCENTRE_SERVUTILS_H
#define CALLCENTRE_SERVUTILS_H

#include <memory>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
using tcp = boost::asio::ip::tcp;

namespace servUtils {
    std::mutex countMutex;

    std::size_t request_count();

//    std::time_t now()
//    {
//        return std::time(0);
//    }
//
//    class commonstructure {
//    public:
//        commonstructure(){}
//    private:
//        int m_callid;
//        http::request<http::dynamic_body> request_;
//
//    };
}

#endif //CALLCENTRE_SERVUTILS_H
