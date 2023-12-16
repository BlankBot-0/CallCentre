//
// Created by vinilco on 11/23/23.
//

#include "CallOperator.h"

ShardedDistribution::ShardedDistribution(size_t shardsCount, int minCallDuration, int maxCallDuration) :
        shardsCount(shardsCount),
        shards(std::vector<Shard>(shardsCount)),
        distribution(std::uniform_int_distribution<>{
                minCallDuration,
                maxCallDuration}
        ) {
}

ShardedDistribution::Shard::Shard() :
        generator(std::chrono::steady_clock::now().time_since_epoch().count()) {
        }

std::chrono::duration<int> ShardedDistribution::getDuration(std::thread::id threadId) {
    size_t index = std::hash<std::thread::id>{}(threadId) % shardsCount;
    auto &shard = shards[index];
    std::lock_guard<std::mutex> lock(shard.mutex);
    return static_cast<std::chrono::duration<int>>(distribution(shard.generator));
}

CallOperator::CallOperator(size_t ID, ShardedDistribution &shardedDistribution) :
        operatorID_(ID),
        available_(true), shardedDistribution(shardedDistribution) {}

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
        BOOST_LOG_TRIVIAL(trace) << "Generating call duration between operator "
                                 << getOperatorID()
                                 << " and client with number "
                                 << clientCall->getNumber();

        std::chrono::duration<int> callDuration = shardedDistribution.getDuration(std::this_thread::get_id());

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