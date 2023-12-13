//
// Created by vinilco on 12/9/23.
//

#include <iomanip>
#include "RequestQueue.h"

Call::Call(size_t callID,
           std::chrono::system_clock::time_point DT_incoming,
           numberType clientNumber) :
           callID_(callID),
           DT_incoming_(DT_incoming),
           callStatus_(CallStatus::PENDING),
           number_(clientNumber) {
    BOOST_LOG_TRIVIAL(trace) << "Call " << callID_ << " constructed";
}

std::string Call::timeToString(const std::chrono::system_clock::time_point &timePoint) const {
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);

    std::tm tmStruct;
    localtime_r(&time, &tmStruct);

    std::ostringstream oss;
    oss << std::put_time(&tmStruct, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

void Call::setCallStatus(CallStatus status) {
    callStatus_ = status;
}

numberType Call::getNumber() const{
    return number_;
}

std::string Call::callStatusToString(CallStatus status) const {
    switch (status) {
        case CallStatus::OK:
            return "OK";
        case CallStatus::PENDING:
            return "PENDING";
        case CallStatus::TIMEOUT:
            return "TIMEOUT";
        case CallStatus::OVERLOAD:
            return "OVERLOAD";
        default:
            return "Unknown";
    }
}

std::string Call::getReport() const {
    std::stringstream report;
    if (callStatus_ == CallStatus::OK) {
        // Format the report
        report << timeToString(DT_incoming_) << " ; "
               << timeToString(DT_answered_) << " ; "
               << timeToString(DT_completion_) << " ; "
               << callDuration_.count() << "s " << " ; "
               << callStatusToString(callStatus_) << " ; "
               << callID_ << " ; "
               << operatorID_ << " ; "
               << std::string(getNumber());
    } else {
        report << timeToString(DT_incoming_) << " ; "
               << callStatusToString(callStatus_) << " ; "
               << callID_ << " ; "
               << std::string(getNumber());
    }
    BOOST_LOG_TRIVIAL(trace) << "Report requested from call " << callID_;

    return report.str();
}

void Call::setStats(std::chrono::system_clock::time_point DT_answered,
                    std::chrono::system_clock::time_point DT_completion,
                    std::chrono::duration<int> callDuration,
                    size_t operatorID) {
    BOOST_LOG_TRIVIAL(trace) << "Status of call " << callID_ << " is OK";

    DT_answered_ = DT_answered;
    DT_completion_ = DT_completion;
    callDuration_ = callDuration;
    operatorID_ = operatorID;

    callStatus_ = CallStatus::OK;
}
RequestQueue::RequestQueue() {}

RequestQueue& RequestQueue::Get() {
    static RequestQueue instance;
    return instance;
}

void RequestQueue::setMaxSize(std::size_t maxSize) {
    maxSize_ = maxSize;
}

void RequestQueue::push(std::unique_ptr<Call> callRequest) {
    BOOST_LOG_TRIVIAL(debug) << "Client number " << callRequest->getNumber() << " pushed into queue";
    numberType number = callRequest->getNumber();

    std::unique_lock lock(mutex_);
    RequestsMap_[number] = std::move(callRequest);
    RequestQueue_.push_back(number);
}

std::unique_ptr<Call> RequestQueue::pop() {
    std::unique_lock lock(mutex_);
    if (RequestQueue_.empty()) {
        return nullptr;
    }

    numberType clientNumber = RequestQueue_.front();
    RequestQueue_.pop_front();

    BOOST_LOG_TRIVIAL(debug) << "Call popped from queue";
    std::unique_ptr<Call> result = std::move(RequestsMap_[clientNumber]);
    RequestsMap_.erase(clientNumber);

    return result;
}

std::unique_ptr<Call> RequestQueue::eraseRequest(numberType clientNumber) {
    std::unique_lock lock(mutex_);
    if (!containsNoLock(clientNumber)) {
        BOOST_LOG_TRIVIAL(warning) << "An attempt to erase non-existing client number " << clientNumber;
        return nullptr;
    }
    BOOST_LOG_TRIVIAL(trace) << "Erasing clent number " << clientNumber << " from queue";
    auto queueIter = RequestQueue_.begin();
    while (*queueIter != clientNumber) {
        ++queueIter;
    }
    RequestQueue_.erase(queueIter);
    BOOST_LOG_TRIVIAL(debug) << "Getting the ownership of the call with client number " << clientNumber;
    std::unique_ptr<Call> req = std::move(RequestsMap_[clientNumber]);
    RequestsMap_.erase(clientNumber);

    return req;
}

bool RequestQueue::isEmpty() {
    std::shared_lock lock(mutex_);
    return RequestQueue_.empty();
}

bool RequestQueue::isFull() {
    std::shared_lock lock(mutex_);
    return RequestQueue_.size() == maxSize_;
}

bool RequestQueue::contains(numberType clientNumber) {
    std::shared_lock lock(mutex_);
    return containsNoLock(clientNumber);
}

bool RequestQueue::containsNoLock(numberType clientNumber) {
    return RequestsMap_.contains(clientNumber);
}