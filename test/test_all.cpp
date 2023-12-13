//
// Created by vinilco on 12/13/23.
//
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../cpp/CallOperator.h"
#include "../cpp/Server.h"

using namespace testing;

// RequestQueue test

class MockCall : public CallInterface {
public:
    MockCall(){}

    // Mocked methods from CallInterface
    MOCK_CONST_METHOD0(getNumber, numberType());
    MOCK_METHOD1(setCallStatus, void(CallStatus status));
    MOCK_CONST_METHOD0(getReport, std::string());
    MOCK_METHOD4(setStats, void(std::chrono::system_clock::time_point,
            std::chrono::system_clock::time_point,
            std::chrono::duration<int>,
            size_t));
};

std::string testTimeToString(const std::chrono::system_clock::time_point &timePoint) {
    std::time_t time = std::chrono::system_clock::to_time_t(timePoint);

    std::tm tmStruct;
    localtime_r(&time, &tmStruct);

    std::ostringstream oss;
    oss << std::put_time(&tmStruct, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

TEST(REQUESTQUEUE, PushPopEmpty) {
    RequestQueue &queue = RequestQueue::Get();
    EXPECT_TRUE(queue.isEmpty());

    auto mockCall = std::make_unique<MockCall>();
    auto call = std::make_unique<Call>(1,
                                       std::chrono::system_clock::now(),
                                       "123");
    queue.push(std::move(call));
    EXPECT_FALSE(queue.isEmpty());

    auto poppedCall = queue.pop();
    EXPECT_TRUE(queue.isEmpty());
    EXPECT_NE(poppedCall, nullptr);
    EXPECT_EQ(poppedCall->getNumber(), "123");
}

TEST(REQUESTQUEUE, SetSize) {
    // Arrange
    RequestQueue &queue = RequestQueue::Get();

    // Act
    queue.setMaxSize(2);
    for (std::size_t i = 0; i < 2; ++i) {
        queue.push(
                std::make_unique<Call>(i,
                                       std::chrono::system_clock::now(),
                                       std::to_string(10 * i))
        );
    }

    // Assert
    EXPECT_TRUE(queue.isFull());
}

// Call tests

TEST(CALLTEST, SetStats) {
    // Arrange
    auto DT_incoming = std::chrono::system_clock::now();
    Call call{1,
              DT_incoming,
              "123"};
    auto DT_answered = std::chrono::system_clock::now();
    auto DT_completion = DT_answered + std::chrono::seconds(10);
    std::chrono::duration<int> callDuration = std::chrono::seconds(10);
    size_t operatorID = 10;

    std::stringstream expectedReport;
    expectedReport << testTimeToString(DT_incoming) << " ; "
                   << testTimeToString(DT_answered) << " ; "
                   << testTimeToString(DT_completion) << " ; "
                   << callDuration.count() << "s " << " ; "
                   << "OK ; "
                   << 1 << " ; "
                   << 10 << " ; "
                   << "123";

    // Act
    call.setStats(DT_answered, DT_completion, callDuration, operatorID);

    // Assert
    ASSERT_STREQ(call.getReport().c_str(), expectedReport.str().c_str());
}