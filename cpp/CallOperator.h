//
// Created by vinilco on 11/23/23.
//

#ifndef CALLCENTRE_CALLOPERATOR_H
#define CALLCENTRE_CALLOPERATOR_H

#include <iostream>
#include <chrono>
#include <thread>
#include <random>

#include "CDRwriter.h"

class CallOperator {
public:
    CallOperator(size_t ID, int minCallDuration, int maxCallDuration);

    void processCalls();
    std::size_t getOperatorID();

private:
    size_t operatorID_;
    bool available_;
    std::uniform_int_distribution<> processTime_;
    // static std::uniform_int_distribution<> processTime_;
    static std::mt19937 generator_;
    static std::mutex mutex_;

};

#endif //CALLCENTRE_CALLOPERATOR_H