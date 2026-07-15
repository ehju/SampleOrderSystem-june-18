#include <gtest/gtest.h>
#include <chrono>

#include "../Model/Order.h"
#include "../Model/OrderStatus.h"

using order_system::Order;
using order_system::OrderStatus;

// 정상 값으로 생성 시 getter 가 입력값을 그대로 반환하며, status 초기값(RESERVED)을 확인한다.
TEST(OrderTest, ConstructsWithValidValuesAndGettersReturnThem)
{
    std::chrono::system_clock::time_point createdAt = std::chrono::system_clock::now();

    Order order(1, 10, "CustomerA", 5, createdAt);

    EXPECT_EQ(order.GetOrderId(), 1);
    EXPECT_EQ(order.GetSampleId(), 10);
    EXPECT_EQ(order.GetCustomerName(), "CustomerA");
    EXPECT_EQ(order.GetQuantity(), 5);
    EXPECT_EQ(order.GetCreatedAt(), createdAt);
}

// 주문은 접수(생성) 시 초기 상태로 RESERVED 를 갖는다.
TEST(OrderTest, InitialStatusIsReserved)
{
    std::chrono::system_clock::time_point createdAt = std::chrono::system_clock::now();

    Order order(1, 10, "CustomerA", 5, createdAt);

    EXPECT_EQ(order.GetStatus(), OrderStatus::RESERVED);
}
