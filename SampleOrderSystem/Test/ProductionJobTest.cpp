#include <gtest/gtest.h>
#include <chrono>

#include "../Model/ProductionJob.h"

using order_system::ProductionJob;

// 정상 값으로 생성 시 getter 가 입력값을 그대로 반환한다.
// 이 phase 에서는 ceil 계산 등 산출 로직은 다루지 않으며, 생성자에 전달된 값을
// 그대로 보관/반환하는지만 검증한다 (계산은 Phase 4/5 승인/생산 로직의 책임).
TEST(ProductionJobTest, ConstructsWithValidValuesAndGettersReturnThem)
{
    std::chrono::system_clock::time_point enqueuedAt = std::chrono::system_clock::now();

    ProductionJob job(1, 10, 20, 25, 750.0, enqueuedAt, 0);

    EXPECT_EQ(job.GetOrderId(), 1);
    EXPECT_EQ(job.GetSampleId(), 10);
    EXPECT_EQ(job.GetShortageQuantity(), 20);
    EXPECT_EQ(job.GetActualProductionQuantity(), 25);
    EXPECT_DOUBLE_EQ(job.GetTotalProductionTime(), 750.0);
    EXPECT_EQ(job.GetEnqueuedAt(), enqueuedAt);
    EXPECT_EQ(job.GetProducedQuantity(), 0);
}
