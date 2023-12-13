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

struct ShardedDistribution {
    ShardedDistribution(size_t shardsCount, int minCallDuration, int maxCallDuration);

    std::chrono::duration<int> getDuration(std::thread::id threadId);

    struct Shard {
        std::mt19937 generator;
        std::mutex mutex;
    };

private:
    std::uniform_int_distribution<> distribution;
    size_t shardsCount;
    std::vector<Shard> shards;
};

class CallOperator {
public:
    CallOperator(size_t ID, ShardedDistribution &distribution);

    void processCalls();
    std::size_t getOperatorID();

private:
    size_t operatorID_;
    bool available_;
    ShardedDistribution &shardedDistribution;
};

#endif //CALLCENTRE_CALLOPERATOR_H