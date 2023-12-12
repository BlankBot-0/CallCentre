//
// Created by vinilco on 12/9/23.
//

#include "RequestQueue.h"

Call::Call(size_t callID,
           std::chrono::system_clock::time_point DT_incoming,
           numberType clientNumber) :
           callID_(callID),
           callStatus_(CallStatus::PENDING),
           number_(clientNumber) {
    BOOST_LOG_TRIVIAL(trace) << "Call " << callID_ << " constructed";
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
        default:
            return "Unknown";
    }
}

std::string Call::getReport() const {
    std::stringstream report;
    BOOST_LOG_TRIVIAL(trace) << "Report requested from call " << callID_;

    // Format the report
    report << std::chrono::system_clock::to_time_t(DT_incoming_) << " ; "
           << std::chrono::system_clock::to_time_t(DT_answered_) << " ; "
           << std::chrono::system_clock::to_time_t(DT_completion_) << " ; "
           << callDuration_.count() << "s " << " ; "
           << callStatusToString(callStatus_) << " ; "
           << callID_ << " ; "
           << operatorID_ << " ; "
           << std::string(getNumber());

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

RequestQueue::RequestQueue() : maxSize_(100) {}

RequestQueue& RequestQueue::Get() {
    static RequestQueue instance;
    return instance;
}

void RequestQueue::push(std::unique_ptr<Call> callRequest) {
    BOOST_LOG_TRIVIAL(debug) << "Call " << callRequest->getNumber() << " pushed into queue";
    numberType number = callRequest->getNumber();

    std::lock_guard<std::mutex> lock(mutex_);
    RequestsMap_[number] = std::move(callRequest);
    RequestQueue_.push_back(number);
}

std::unique_ptr<Call> RequestQueue::pop() {
    std::lock_guard<std::mutex> lock(mutex_);
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
    if (!contains(clientNumber)) {
        BOOST_LOG_TRIVIAL(warning) << "An attempt to erase non-existing client number " << clientNumber;
        return nullptr;
    }
    BOOST_LOG_TRIVIAL(trace) << "Erasing clent number " << clientNumber << " from queue";
    std::lock_guard<std::mutex> lock(mutex_);
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
    std::lock_guard<std::mutex> lock(mutex_);
    return RequestQueue_.empty();
}

bool RequestQueue::isFull() {
    std::lock_guard<std::mutex> lock(mutex_);
    return RequestQueue_.size() == maxSize_;
}

bool RequestQueue::contains(numberType clientNumber) {
    std::lock_guard<std::mutex> lock(mutex_);
    return RequestsMap_.contains(clientNumber);
}