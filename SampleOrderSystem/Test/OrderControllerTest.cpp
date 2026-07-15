#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <optional>

#include "../Controller/OrderController.h"
#include "MockOrderRepository.h"
#include "MockSampleRepository.h"

using ::testing::_;
using ::testing::Return;
using order_system::MockOrderRepository;
using order_system::MockSampleRepository;
using order_system::Order;
using order_system::OrderApprovalResult;
using order_system::OrderController;
using order_system::OrderReservationResult;
using order_system::OrderStatus;
using order_system::Sample;

namespace {

Sample MakeSample(int sampleId)
{
    return Sample(sampleId, "SampleA", 30, 0.9, 0);
}

Sample MakeSampleWithStock(int sampleId, int avgProductionTime, double yield, int stockQuantity)
{
    return Sample(sampleId, "SampleA", avgProductionTime, yield, stockQuantity);
}

Order MakeReservedOrder(int orderId, int sampleId, int quantity)
{
    return Order(orderId, sampleId, "CustomerA", quantity, std::chrono::system_clock::now());
}

Order MakeOrderWithStatus(int orderId, int sampleId, int quantity, OrderStatus status)
{
    Order order(orderId, sampleId, "CustomerA", quantity, std::chrono::system_clock::now());
    order.SetStatus(status);
    return order;
}

}  // namespace

// 재고가 충분하면 주문 상태가 CONFIRMED 로 전환되고, 재고는 차감되지 않는다 (정책 B).
TEST(OrderControllerTest, ApprovesOrderAsConfirmedWhenStockIsSufficient)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(1)).WillOnce(Return(MakeReservedOrder(1, 10, 5)));
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSampleWithStock(10, 30, 0.9, 5)));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(1, OrderStatus::CONFIRMED)).Times(1);
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockSampleRepository, IncreaseStock(_, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderApprovalResult result = controller.ApproveOrder(1);

    EXPECT_TRUE(result.success);
    ASSERT_TRUE(result.newStatus.has_value());
    EXPECT_EQ(*result.newStatus, OrderStatus::CONFIRMED);
}

// 재고가 부족하면 ProductionJob 이 생성되어 생산 큐에 추가되고, 주문 상태는 PRODUCING 으로 전환된다.
// 이 시점에도 재고는 차감되지 않는다.
TEST(OrderControllerTest, ApprovesOrderAsProducingWhenStockIsInsufficient)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(2)).WillOnce(Return(MakeReservedOrder(2, 10, 20)));
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSampleWithStock(10, 30, 0.9, 5)));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(2, OrderStatus::PRODUCING)).Times(1);
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockSampleRepository, IncreaseStock(_, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderApprovalResult result = controller.ApproveOrder(2);

    EXPECT_TRUE(result.success);
    ASSERT_TRUE(result.newStatus.has_value());
    EXPECT_EQ(*result.newStatus, OrderStatus::PRODUCING);

    ASSERT_EQ(controller.GetProductionQueue().size(), 1u);
    const order_system::ProductionJob& job = controller.GetProductionQueue().front();
    EXPECT_EQ(job.GetOrderId(), 2);
    EXPECT_EQ(job.GetSampleId(), 10);
    EXPECT_EQ(job.GetShortageQuantity(), 15);  // quantity(20) - stock(5)
}

// shortageQuantity/yield 가 나누어 떨어지지 않는 경우 ceil 로 올림 계산한다.
// shortageQuantity=10, yield=0.9 -> ceil(11.11..) = 12.
TEST(OrderControllerTest, ComputesActualProductionQuantityWithCeilWhenNotDivisible)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(3)).WillOnce(Return(MakeReservedOrder(3, 10, 10)));
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSampleWithStock(10, 30, 0.9, 0)));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(3, OrderStatus::PRODUCING)).Times(1);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    controller.ApproveOrder(3);

    ASSERT_EQ(controller.GetProductionQueue().size(), 1u);
    const order_system::ProductionJob& job = controller.GetProductionQueue().front();
    EXPECT_EQ(job.GetShortageQuantity(), 10);
    EXPECT_EQ(job.GetActualProductionQuantity(), 12);
    EXPECT_DOUBLE_EQ(job.GetTotalProductionTime(), 30 * 12);
}

// shortageQuantity/yield 가 정확히 나누어 떨어지는 경우 ceil 결과가 그대로 유지된다.
TEST(OrderControllerTest, ComputesActualProductionQuantityWithCeilWhenDivisible)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(4)).WillOnce(Return(MakeReservedOrder(4, 10, 10)));
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSampleWithStock(10, 30, 0.5, 0)));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(4, OrderStatus::PRODUCING)).Times(1);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    controller.ApproveOrder(4);

    ASSERT_EQ(controller.GetProductionQueue().size(), 1u);
    const order_system::ProductionJob& job = controller.GetProductionQueue().front();
    EXPECT_EQ(job.GetShortageQuantity(), 10);
    EXPECT_EQ(job.GetActualProductionQuantity(), 20);
    EXPECT_DOUBLE_EQ(job.GetTotalProductionTime(), 30 * 20);
}

// 존재하지 않는 주문에 대한 승인 시도는 거부된다.
TEST(OrderControllerTest, RejectsApprovalWhenOrderDoesNotExist)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(999)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderApprovalResult result = controller.ApproveOrder(999);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "승인 가능한 주문이 아닙니다");
    EXPECT_FALSE(result.newStatus.has_value());
}

// 이미 처리된(RESERVED 가 아닌) 주문에 대한 승인 재시도는 거부된다.
TEST(OrderControllerTest, RejectsApprovalRetryForAlreadyProcessedOrder)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(5))
        .WillOnce(Return(MakeOrderWithStatus(5, 10, 5, OrderStatus::CONFIRMED)));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderApprovalResult result = controller.ApproveOrder(5);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "승인 가능한 주문이 아닙니다");
}

// RESERVED 상태의 주문을 거절하면 REJECTED 로 전환되고, 재고/큐에는 영향이 없다.
TEST(OrderControllerTest, RejectsReservedOrder)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(6)).WillOnce(Return(MakeReservedOrder(6, 10, 5)));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(6, OrderStatus::REJECTED)).Times(1);
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockSampleRepository, IncreaseStock(_, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderApprovalResult result = controller.RejectOrder(6);

    EXPECT_TRUE(result.success);
    ASSERT_TRUE(result.newStatus.has_value());
    EXPECT_EQ(*result.newStatus, OrderStatus::REJECTED);
    EXPECT_EQ(controller.GetProductionQueue().size(), 0u);
}

// 이미 처리된(RESERVED 가 아닌) 주문에 대한 거절 재시도는 거부된다.
TEST(OrderControllerTest, RejectsRejectionRetryForAlreadyProcessedOrder)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(7))
        .WillOnce(Return(MakeOrderWithStatus(7, 10, 5, OrderStatus::REJECTED)));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderApprovalResult result = controller.RejectOrder(7);

    EXPECT_FALSE(result.success);
}

// 존재하지 않는 주문에 대한 거절 시도는 거부된다.
TEST(OrderControllerTest, RejectsRejectionWhenOrderDoesNotExist)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(998)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderApprovalResult result = controller.RejectOrder(998);

    EXPECT_FALSE(result.success);
}

// GetReservedOrders 는 RESERVED 상태 주문만 createdAt 오름차순(FIFO)으로 반환한다.
TEST(OrderControllerTest, GetReservedOrdersReturnsOnlyReservedOrdersSortedByCreatedAt)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    std::chrono::system_clock::time_point earlier = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point later = earlier + std::chrono::seconds(10);
    Order olderOrder(1, 10, "CustomerA", 5, earlier);
    Order newerOrder(2, 10, "CustomerB", 3, later);
    EXPECT_CALL(mockOrderRepository, FindByStatus(OrderStatus::RESERVED))
        .WillOnce(Return(std::vector<Order>{ newerOrder, olderOrder }));

    OrderController controller(mockSampleRepository, mockOrderRepository);
    std::vector<Order> reservedOrders = controller.GetReservedOrders();

    ASSERT_EQ(reservedOrders.size(), 2u);
    EXPECT_EQ(reservedOrders[0].GetOrderId(), 1);
    EXPECT_EQ(reservedOrders[1].GetOrderId(), 2);
}

// 존재하지 않는 시료 ID 로 주문하면 등록을 거부한다.
TEST(OrderControllerTest, RejectsWhenSampleDoesNotExist)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, FindById(999)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockOrderRepository, Add(_, _, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderReservationResult result = controller.ReserveOrder(999, "CustomerA", 5);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "존재하지 않는 시료입니다");
}

// 고객명이 빈 문자열이면 등록을 거부한다.
TEST(OrderControllerTest, RejectsEmptyCustomerName)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSample(10)));
    EXPECT_CALL(mockOrderRepository, Add(_, _, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderReservationResult result = controller.ReserveOrder(10, "", 5);

    EXPECT_FALSE(result.success);
}

// 고객명이 공백만으로 구성되어 있으면 등록을 거부한다.
TEST(OrderControllerTest, RejectsBlankCustomerName)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSample(10)));
    EXPECT_CALL(mockOrderRepository, Add(_, _, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderReservationResult result = controller.ReserveOrder(10, "   ", 5);

    EXPECT_FALSE(result.success);
}

// 주문 수량이 0이면 등록을 거부한다.
TEST(OrderControllerTest, RejectsQuantityOfZero)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSample(10)));
    EXPECT_CALL(mockOrderRepository, Add(_, _, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderReservationResult result = controller.ReserveOrder(10, "CustomerA", 0);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "주문 수량은 1 이상이어야 합니다");
}

// 주문 수량이 음수이면 등록을 거부한다.
TEST(OrderControllerTest, RejectsNegativeQuantity)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSample(10)));
    EXPECT_CALL(mockOrderRepository, Add(_, _, _)).Times(0);

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderReservationResult result = controller.ReserveOrder(10, "CustomerA", -1);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "주문 수량은 1 이상이어야 합니다");
}

// 정상 입력이면 OrderRepository 의 생성 메서드가 호출되고, 결과로 status = RESERVED 인 주문이 반환된다.
TEST(OrderControllerTest, ReservesOrderWhenInputIsValid)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    std::chrono::system_clock::time_point createdAt = std::chrono::system_clock::now();
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSample(10)));
    EXPECT_CALL(mockOrderRepository, Add(10, "CustomerA", 5))
        .WillOnce(Return(Order(1, 10, "CustomerA", 5, createdAt)));

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderReservationResult result = controller.ReserveOrder(10, "CustomerA", 5);

    EXPECT_TRUE(result.success);
    ASSERT_TRUE(result.order.has_value());
    EXPECT_EQ(result.order->GetOrderId(), 1);
    EXPECT_EQ(result.order->GetStatus(), OrderStatus::RESERVED);
}

// 동일 고객이 동일 시료를 중복 주문해도 각각 별도의 주문으로 접수된다(제한하지 않음).
TEST(OrderControllerTest, AllowsDuplicateOrderFromSameCustomerForSameSample)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    std::chrono::system_clock::time_point createdAt = std::chrono::system_clock::now();
    EXPECT_CALL(mockSampleRepository, FindById(10)).Times(2).WillRepeatedly(Return(MakeSample(10)));
    EXPECT_CALL(mockOrderRepository, Add(10, "CustomerA", 5))
        .WillOnce(Return(Order(1, 10, "CustomerA", 5, createdAt)))
        .WillOnce(Return(Order(2, 10, "CustomerA", 5, createdAt)));

    OrderController controller(mockSampleRepository, mockOrderRepository);
    OrderReservationResult firstResult = controller.ReserveOrder(10, "CustomerA", 5);
    OrderReservationResult secondResult = controller.ReserveOrder(10, "CustomerA", 5);

    EXPECT_TRUE(firstResult.success);
    EXPECT_TRUE(secondResult.success);
}
