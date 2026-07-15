#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <vector>

#include "../Controller/MonitoringController.h"
#include "MockOrderRepository.h"
#include "MockSampleRepository.h"

using ::testing::Return;
using order_system::MockOrderRepository;
using order_system::MockSampleRepository;
using order_system::MonitoringController;
using order_system::Order;
using order_system::OrderStatus;
using order_system::OrderStatusCounts;
using order_system::Sample;
using order_system::SampleStockStatusRow;
using order_system::StockStatus;

namespace {

Order MakeOrder(int orderId, int sampleId, int quantity, OrderStatus status)
{
    Order order(orderId, sampleId, "CustomerA", quantity, std::chrono::system_clock::now());
    order.SetStatus(status);
    return order;
}

Sample MakeSample(int sampleId, int stockQuantity)
{
    return Sample(sampleId, "WaferA", 30, 0.9, stockQuantity);
}

}  // namespace

// 상태별 주문 개수: RESERVED/PRODUCING/CONFIRMED/RELEASE 각각 정확히 계산되고 REJECTED 는 제외한다.
TEST(MonitoringControllerTest, CountsOrdersByStatusExcludingRejected)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    std::vector<Order> orders{
        MakeOrder(1, 10, 5, OrderStatus::RESERVED),
        MakeOrder(2, 10, 5, OrderStatus::RESERVED),
        MakeOrder(3, 10, 5, OrderStatus::RESERVED),
        MakeOrder(4, 10, 5, OrderStatus::PRODUCING),
        MakeOrder(5, 10, 5, OrderStatus::PRODUCING),
        MakeOrder(6, 10, 5, OrderStatus::CONFIRMED),
        MakeOrder(7, 10, 5, OrderStatus::CONFIRMED),
        MakeOrder(8, 10, 5, OrderStatus::CONFIRMED),
        MakeOrder(9, 10, 5, OrderStatus::CONFIRMED),
        MakeOrder(10, 10, 5, OrderStatus::RELEASE),
        MakeOrder(11, 10, 5, OrderStatus::REJECTED),
    };
    EXPECT_CALL(mockOrderRepository, GetAll()).WillOnce(Return(orders));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    OrderStatusCounts counts = controller.GetOrderStatusCounts();

    EXPECT_EQ(counts.reservedCount, 3);
    EXPECT_EQ(counts.producingCount, 2);
    EXPECT_EQ(counts.confirmedCount, 4);
    EXPECT_EQ(counts.releaseCount, 1);
}

// 주문이 하나도 없으면 모든 상태 카운트가 0이다.
TEST(MonitoringControllerTest, ReturnsAllZeroCountsWhenNoOrders)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, GetAll()).WillOnce(Return(std::vector<Order>{}));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    OrderStatusCounts counts = controller.GetOrderStatusCounts();

    EXPECT_EQ(counts.reservedCount, 0);
    EXPECT_EQ(counts.producingCount, 0);
    EXPECT_EQ(counts.confirmedCount, 0);
    EXPECT_EQ(counts.releaseCount, 0);
}

// 재고 상태 경계값: stockQuantity == 대기수량합 이면 "여유" 로 판정한다.
TEST(MonitoringControllerTest, JudgesStockAsSufficientWhenStockEqualsPendingSum)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, GetAll()).WillOnce(Return(std::vector<Sample>{ MakeSample(1, 30) }));
    EXPECT_CALL(mockOrderRepository, GetAll())
        .WillOnce(Return(std::vector<Order>{
            MakeOrder(1, 1, 10, OrderStatus::RESERVED),
            MakeOrder(2, 1, 10, OrderStatus::PRODUCING),
            MakeOrder(3, 1, 10, OrderStatus::CONFIRMED),
        }));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    std::vector<SampleStockStatusRow> rows = controller.GetStockStatuses();

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].sampleId, 1);
    EXPECT_EQ(rows[0].stockQuantity, 30);
    EXPECT_EQ(rows[0].pendingQuantity, 30);
    EXPECT_EQ(rows[0].status, StockStatus::SUFFICIENT);
}

// 재고 상태 경계값: stockQuantity == 0 이고 대기수량합 == 0 이면 "고갈" 로 판정한다.
TEST(MonitoringControllerTest, JudgesStockAsDepletedWhenStockIsZeroAndNoPendingOrders)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, GetAll()).WillOnce(Return(std::vector<Sample>{ MakeSample(1, 0) }));
    EXPECT_CALL(mockOrderRepository, GetAll()).WillOnce(Return(std::vector<Order>{}));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    std::vector<SampleStockStatusRow> rows = controller.GetStockStatuses();

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].stockQuantity, 0);
    EXPECT_EQ(rows[0].pendingQuantity, 0);
    EXPECT_EQ(rows[0].status, StockStatus::DEPLETED);
}

// 재고 상태 경계값: stockQuantity == 0 이고 대기수량합 > 0 이어도 "고갈" 이 우선한다.
TEST(MonitoringControllerTest, JudgesStockAsDepletedWhenStockIsZeroEvenIfPendingOrdersExist)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, GetAll()).WillOnce(Return(std::vector<Sample>{ MakeSample(2, 0) }));
    EXPECT_CALL(mockOrderRepository, GetAll())
        .WillOnce(Return(std::vector<Order>{ MakeOrder(1, 2, 10, OrderStatus::RESERVED) }));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    std::vector<SampleStockStatusRow> rows = controller.GetStockStatuses();

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].stockQuantity, 0);
    EXPECT_EQ(rows[0].pendingQuantity, 10);
    EXPECT_EQ(rows[0].status, StockStatus::DEPLETED);
}

// 재고 상태 경계값: 0 < stockQuantity < 대기수량합 이면 "부족" 으로 판정한다.
TEST(MonitoringControllerTest, JudgesStockAsShortageWhenStockIsPositiveButLessThanPendingSum)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, GetAll()).WillOnce(Return(std::vector<Sample>{ MakeSample(3, 5) }));
    EXPECT_CALL(mockOrderRepository, GetAll())
        .WillOnce(Return(std::vector<Order>{ MakeOrder(1, 3, 20, OrderStatus::RESERVED) }));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    std::vector<SampleStockStatusRow> rows = controller.GetStockStatuses();

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].stockQuantity, 5);
    EXPECT_EQ(rows[0].pendingQuantity, 20);
    EXPECT_EQ(rows[0].status, StockStatus::SHORTAGE);
}

// 재고 상태 경계값: stockQuantity > 대기수량합 이면 "여유" 로 판정한다.
TEST(MonitoringControllerTest, JudgesStockAsSufficientWhenStockExceedsPendingSum)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, GetAll()).WillOnce(Return(std::vector<Sample>{ MakeSample(1, 50) }));
    EXPECT_CALL(mockOrderRepository, GetAll())
        .WillOnce(Return(std::vector<Order>{ MakeOrder(1, 1, 30, OrderStatus::RESERVED) }));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    std::vector<SampleStockStatusRow> rows = controller.GetStockStatuses();

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].stockQuantity, 50);
    EXPECT_EQ(rows[0].pendingQuantity, 30);
    EXPECT_EQ(rows[0].status, StockStatus::SUFFICIENT);
}

// 대기수량합 계산에는 RESERVED/PRODUCING/CONFIRMED 만 포함하고, RELEASE/REJECTED 는 제외한다.
TEST(MonitoringControllerTest, ExcludesReleaseAndRejectedOrdersFromPendingSum)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, GetAll()).WillOnce(Return(std::vector<Sample>{ MakeSample(4, 100) }));
    EXPECT_CALL(mockOrderRepository, GetAll())
        .WillOnce(Return(std::vector<Order>{
            MakeOrder(1, 4, 10, OrderStatus::RESERVED),
            MakeOrder(2, 4, 1000, OrderStatus::RELEASE),
            MakeOrder(3, 4, 1000, OrderStatus::REJECTED),
        }));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    std::vector<SampleStockStatusRow> rows = controller.GetStockStatuses();

    ASSERT_EQ(rows.size(), 1u);
    EXPECT_EQ(rows[0].pendingQuantity, 10);
    EXPECT_EQ(rows[0].status, StockStatus::SUFFICIENT);
}

// 시료가 하나도 없으면 재고 현황 목록이 비어 있다.
TEST(MonitoringControllerTest, ReturnsEmptyStockStatusesWhenNoSamples)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    ON_CALL(mockOrderRepository, GetAll()).WillByDefault(Return(std::vector<Order>{}));
    EXPECT_CALL(mockSampleRepository, GetAll()).WillOnce(Return(std::vector<Sample>{}));

    MonitoringController controller(mockSampleRepository, mockOrderRepository);
    std::vector<SampleStockStatusRow> rows = controller.GetStockStatuses();

    EXPECT_TRUE(rows.empty());
}
