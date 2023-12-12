//
// Created by vinilco on 11/23/23.
//

#include "CallOperator.h"

CallOperator::CallOperator(size_t ID, int minCallDuration, int maxCallDuration) :
        operatorID_(ID),
        available_(true),
        processTime_{minCallDuration, maxCallDuration}
        {}

std::size_t CallOperator::getOperatorID() {
    return operatorID_;
}

void CallOperator::processCalls() {
    while (true) {
        auto clientCall = RequestQueue::Get().pop();
        if (!clientCall) {
            // BOOST_LOG_TRIVIAL(trace) << "Operator " << getOperatorID() << " awaits new calls";
            std::this_thread::sleep_for(std::chrono::seconds(3));
            continue;
        }
        available_ = false;
        std::chrono::duration<int> callDuration;
        { // scope for safely accessing rnd num generator
            BOOST_LOG_TRIVIAL(trace) << "Generating call duration between operator "
                                     << getOperatorID()
                                     << " and client with number "
                                     << clientCall->getNumber();

            std::lock_guard<std::mutex> lock(mutex_);
            callDuration = std::chrono::duration<int>{processTime_(generator_)};
        }

        clientCall->setStats(std::chrono::system_clock::now(),
                                   std::chrono::system_clock::now() + callDuration,
                                   callDuration,
                                   operatorID_);

        std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::seconds>(callDuration));
        available_ = true;
        BOOST_LOG_TRIVIAL(trace) << "Writing statistics on client with number "
                                 << clientCall->getNumber()
                                 << " to CDR";
        CDRWriter::getInstance().writeToCDR(clientCall->getReport());
    }
}

std::mt19937 CallOperator::generator_;
std::mutex CallOperator::mutex_;
// TODO: processTime is defined in "config" thus should be initialized in main.cpp