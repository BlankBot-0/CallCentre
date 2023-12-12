//
// Created by vinilco on 12/5/23.
//

#include "servUtils.h"

std::size_t servUtils::request_count() {
    std::lock_guard<std::mutex> lock(servUtils::countMutex);
    static std::size_t count = 0;
    return ++count;
}