#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <optional>

#include "../Controller/ShipmentController.h"
#include "MockOrderRepository.h"
#include "MockSampleRepository.h"

using ::testing::_;
using ::testing::Return;
using order_system::MockOrderRepository;
using order_system::MockSampleRepository;
using order_system::Order;
using order_system::OrderStatus;
using order_system::Sample;
using order_system::ShipmentController;
using order_system::ShipmentResult;

namespace {

Order MakeOrderWithStatus(int orderId, int sampleId, int quantity, OrderStatus status)
{
    Order order(orderId, sampleId, "CustomerA", quantity, std::chrono::system_clock::now());
    order.SetStatus(status);
    return order;
}

Sample MakeSampleWithStock(int sampleId, int stockQuantity)
{
    return Sample(sampleId, "SampleA", 30, 0.9, stockQuantity);
}

}  // namespace

// GetShipmentTargets 는 CONFIRMED 상태 주문만 출고 대상으로 반환한다.
TEST(ShipmentControllerTest, GetShipmentTargetsReturnsOnlyConfirmedOrders)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    std::vector<Order> confirmedOrders = {
        MakeOrderWithStatus(1, 10, 5, OrderStatus::CONFIRMED),
        MakeOrderWithStatus(2, 11, 3, OrderStatus::CONFIRMED),
    };
    EXPECT_CALL(mockOrderRepository, FindByStatus(OrderStatus::CONFIRMED)).WillOnce(Return(confirmedOrders));

    ShipmentController controller(mockSampleRepository, mockOrderRepository);
    std::vector<Order> shipmentTargets = controller.GetShipmentTargets();

    ASSERT_EQ(shipmentTargets.size(), 2u);
    EXPECT_EQ(shipmentTargets[0].GetOrderId(), 1);
    EXPECT_EQ(shipmentTargets[1].GetOrderId(), 2);
}

// CONFIRMED 상태이고 재고가 충분하면 재고가 quantity 만큼 차감되고 주문 상태가 RELEASE 로 전환된다.
TEST(ShipmentControllerTest, ProcessesShipmentSuccessfullyWhenStockIsSufficient)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(1))
        .WillOnce(Return(MakeOrderWithStatus(1, 10, 5, OrderStatus::CONFIRMED)));
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSampleWithStock(10, 5)));
    EXPECT_CALL(mockSampleRepository, DecreaseStock(10, 5)).Times(1);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(1, OrderStatus::RELEASE)).Times(1);

    ShipmentController controller(mockSampleRepository, mockOrderRepository);
    ShipmentResult result = controller.ProcessShipment(1);

    EXPECT_TRUE(result.success);
}

// CONFIRMED 상태이지만 그 사이 다른 주문의 출고로 재고가 줄어들어 stockQuantity < quantity 인 경우,
// 재고 차감 없이 처리를 거부하고 주문 상태는 CONFIRMED 로 유지된다.
TEST(ShipmentControllerTest, RejectsShipmentWhenStockIsInsufficient)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(2))
        .WillOnce(Return(MakeOrderWithStatus(2, 10, 5, OrderStatus::CONFIRMED)));
    EXPECT_CALL(mockSampleRepository, FindById(10)).WillOnce(Return(MakeSampleWithStock(10, 3)));
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    ShipmentController controller(mockSampleRepository, mockOrderRepository);
    ShipmentResult result = controller.ProcessShipment(2);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "재고가 부족하여 출고할 수 없습니다");
}

// RESERVED 상태의 주문에 대한 출고 시도는 거부된다.
TEST(ShipmentControllerTest, RejectsShipmentWhenOrderIsReserved)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(3))
        .WillOnce(Return(MakeOrderWithStatus(3, 10, 5, OrderStatus::RESERVED)));
    EXPECT_CALL(mockSampleRepository, FindById(_)).Times(0);
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    ShipmentController controller(mockSampleRepository, mockOrderRepository);
    ShipmentResult result = controller.ProcessShipment(3);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "출고 가능한 주문이 아닙니다");
}

// PRODUCING 상태의 주문에 대한 출고 시도는 거부된다.
TEST(ShipmentControllerTest, RejectsShipmentWhenOrderIsProducing)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(4))
        .WillOnce(Return(MakeOrderWithStatus(4, 10, 5, OrderStatus::PRODUCING)));
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    ShipmentController controller(mockSampleRepository, mockOrderRepository);
    ShipmentResult result = controller.ProcessShipment(4);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "출고 가능한 주문이 아닙니다");
}

// REJECTED 상태의 주문에 대한 출고 시도는 거부된다.
TEST(ShipmentControllerTest, RejectsShipmentWhenOrderIsRejected)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(5))
        .WillOnce(Return(MakeOrderWithStatus(5, 10, 5, OrderStatus::REJECTED)));
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    ShipmentController controller(mockSampleRepository, mockOrderRepository);
    ShipmentResult result = controller.ProcessShipment(5);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "출고 가능한 주문이 아닙니다");
}

// 이미 RELEASE 상태인 주문에 대한 재출고 시도는 거부된다.
TEST(ShipmentControllerTest, RejectsShipmentWhenOrderIsAlreadyReleased)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(6))
        .WillOnce(Return(MakeOrderWithStatus(6, 10, 5, OrderStatus::RELEASE)));
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    ShipmentController controller(mockSampleRepository, mockOrderRepository);
    ShipmentResult result = controller.ProcessShipment(6);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "출고 가능한 주문이 아닙니다");
}

// 존재하지 않는 주문에 대한 출고 시도는 거부된다.
TEST(ShipmentControllerTest, RejectsShipmentWhenOrderDoesNotExist)
{
    MockSampleRepository mockSampleRepository;
    MockOrderRepository mockOrderRepository;
    EXPECT_CALL(mockOrderRepository, FindById(999)).WillOnce(Return(std::nullopt));
    EXPECT_CALL(mockSampleRepository, DecreaseStock(_, _)).Times(0);
    EXPECT_CALL(mockOrderRepository, UpdateStatus(_, _)).Times(0);

    ShipmentController controller(mockSampleRepository, mockOrderRepository);
    ShipmentResult result = controller.ProcessShipment(999);

    EXPECT_FALSE(result.success);
    EXPECT_EQ(result.errorMessage, "출고 가능한 주문이 아닙니다");
}
