//
// Created by vinilco on 12/9/23.
//

#ifndef PROJECTNAME_REQUESTQUEUE_H
#define PROJECTNAME_REQUESTQUEUE_H

#include <memory>
#include <chrono>
#include <map>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
// #include <string>

enum CallStatus {OK, PENDING, TIMEOUT, OVERLOAD};

using numberType = std::string;
using numberQueue = std::list<numberType>;

class Call
{
public:
    Call(size_t callID,
         std::chrono::system_clock::time_point DT_incoming,
         numberType clientNumber);

    void setCallStatus(CallStatus status);

    numberType getNumber() const;
    void setStats(std::chrono::system_clock::time_point DT_answered,
                  std::chrono::system_clock::time_point DT_completion,
                  std::chrono::duration<int> callDuration,
                  size_t operatorID);
    std::string getReport() const;
private:
    std::string callStatusToString(CallStatus status) const;

    size_t callID_;
    size_t operatorID_;
    numberType number_;

    CallStatus callStatus_;

    std::chrono::system_clock::time_point DT_incoming_,
                                          DT_answered_,
                                          DT_completion_;
    std::chrono::duration<int> callDuration_;
};

class RequestQueue
{
public:
    static RequestQueue& Get();

    void push(std::unique_ptr<Call> data);
    std::unique_ptr<Call> pop();
    std::unique_ptr<Call> eraseRequest(numberType clientNumber);

    bool isEmpty();
    bool isFull();
    bool contains(numberType clientNumber);
    bool containsNoLock(numberType clientNumber);

private:
    RequestQueue();
    std::map<numberType, std::unique_ptr<Call>> RequestsMap_;
    numberQueue RequestQueue_;
    std::shared_mutex mutex_;
    const std::size_t maxSize_;

    RequestQueue(const RequestQueue&) = delete;
    RequestQueue& operator=(const RequestQueue&) = delete;
    RequestQueue(RequestQueue &&) = delete;
    RequestQueue& operator=(RequestQueue&&) = delete;
};

#endif //PROJECTNAME_REQUESTQUEUE_H