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
using order_system::OrderController;
using order_system::OrderReservationResult;
using order_system::OrderStatus;
using order_system::Sample;

namespace {

Sample MakeSample(int sampleId)
{
    return Sample(sampleId, "SampleA", 30, 0.9, 0);
}

}  // namespace

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
