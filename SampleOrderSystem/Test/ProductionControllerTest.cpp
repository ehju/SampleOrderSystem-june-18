#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <optional>

#include "../Controller/ProductionController.h"
#include "../Model/ProductionQueue.h"
#include "MockOrderRepository.h"
#include "MockSampleRepository.h"

using ::testing::_;
using ::testing::Return;
using order_system::MockOrderRepository;
using order_system::MockSampleRepository;
using order_system::Order;
using order_system::OrderStatus;
using order_system::ProductionCompletionResult;
using order_system::ProductionController;
using order_system::ProductionJob;
using order_system::ProductionQueue;

namespace {

ProductionJob MakeJob(int orderId, int sampleId, int shortageQuantity, int actualProductionQuantity,
                       double totalProductionTime)
{
    return ProductionJob(orderId, sampleId, shortageQuantity, actualProductionQuantity, totalProductionTime,
                          std::chrono::system_clock::now(), 0);
}

}  // namespace

// 생산 완료 처리 시 큐 선두 Job 이 제거되고, 대상 Sample 재고가 actualProductionQuantity 만큼 증가하며,
// 대상 Order 상태가 PRODUCING -> CONFIRMED 로 전환된다.
TEST(ProductionControllerTest, CompletesFrontJobAndUpdatesStockAndOrderStatus)
{
    ProductionQueue productionQueue;
    productionQueue.Enqueue(MakeJob(1, 10, 15, 17, 510.0));
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, IncreaseStock(10, 17)).Times(1);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(1, OrderStatus::CONFIRMED)).Times(1);

    ProductionController controller(productionQueue, mockSampleRepository, mockOrderRepository);
    ProductionCompletionResult result = controller.CompleteProduction();

    EXPECT_TRUE(result.success);
    ASSERT_TRUE(result.completedJob.has_value());
    EXPECT_EQ(result.completedJob->GetOrderId(), 1);
    EXPECT_TRUE(productionQueue.IsEmpty());
}

// 여러 건의 PRODUCING 주문이 큐에 등록된 순서대로(FIFO) 완료 처리된다.
TEST(ProductionControllerTest, CompletesMultipleJobsInEnqueuedOrder)
{
    ProductionQueue productionQueue;
    productionQueue.Enqueue(MakeJob(1, 10, 15, 17, 510.0));
    productionQueue.Enqueue(MakeJob(2, 11, 5, 6, 180.0));
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, IncreaseStock(10, 17)).Times(1);
    EXPECT_CALL(mockSampleRepository, IncreaseStock(11, 6)).Times(1);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(1, OrderStatus::CONFIRMED)).Times(1);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(2, OrderStatus::CONFIRMED)).Times(1);

    ProductionController controller(productionQueue, mockSampleRepository, mockOrderRepository);
    ProductionCompletionResult firstResult = controller.CompleteProduction();
    ProductionCompletionResult secondResult = controller.CompleteProduction();

    EXPECT_TRUE(firstResult.success);
    ASSERT_TRUE(firstResult.completedJob.has_value());
    EXPECT_EQ(firstResult.completedJob->GetOrderId(), 1);

    EXPECT_TRUE(secondResult.success);
    ASSERT_TRUE(secondResult.completedJob.has_value());
    EXPECT_EQ(secondResult.completedJob->GetOrderId(), 2);

    EXPECT_TRUE(productionQueue.IsEmpty());
}

// 동일 시료를 대상으로 하는 여러 Job 이 큐에 있어도 각 Job 은 독립적으로 순서대로 처리된다
// (재고 증가/상태 전환이 Job 개수만큼 개별 호출된다).
TEST(ProductionControllerTest, ProcessesEachJobIndependentlyEvenForSameSample)
{
    ProductionQueue productionQueue;
    productionQueue.Enqueue(MakeJob(1, 10, 5, 6, 180.0));
    productionQueue.Enqueue(MakeJob(2, 10, 3, 4, 120.0));
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, IncreaseStock(10, 6)).Times(1);
    EXPECT_CALL(mockSampleRepository, IncreaseStock(10, 4)).Times(1);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(1, OrderStatus::CONFIRMED)).Times(1);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(2, OrderStatus::CONFIRMED)).Times(1);

    ProductionController controller(productionQueue, mockSampleRepository, mockOrderRepository);
    controller.CompleteProduction();
    controller.CompleteProduction();

    EXPECT_TRUE(productionQueue.IsEmpty());
}

// 빈 큐에서 완료 처리 명령을 실행하면 거부되고 안내 메시지가 담긴다.
TEST(ProductionControllerTest, RejectsCompletionWhenQueueIsEmpty)
{
    ProductionQueue productionQueue;
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, IncreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    ProductionController controller(productionQueue, mockSampleRepository, mockOrderRepository);
    ProductionCompletionResult result = controller.CompleteProduction();

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "진행 중인 생산이 없습니다");
    EXPECT_FALSE(result.completedJob.has_value());
}
