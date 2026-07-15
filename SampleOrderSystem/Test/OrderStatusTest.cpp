#include <gtest/gtest.h>

#include "../Model/OrderStatus.h"

using order_system::OrderStatus;

// OrderStatus 는 enum class 이며 5개 상태(RESERVED, REJECTED, PRODUCING, CONFIRMED, RELEASE)를
// 모두 정의하고 있어야 하고, 값 간 비교 연산이 정상 동작해야 한다.
TEST(OrderStatusTest, HasAllFiveDefinedValues)
{
    OrderStatus reserved = OrderStatus::RESERVED;
    OrderStatus rejected = OrderStatus::REJECTED;
    OrderStatus producing = OrderStatus::PRODUCING;
    OrderStatus confirmed = OrderStatus::CONFIRMED;
    OrderStatus release = OrderStatus::RELEASE;

    EXPECT_NE(reserved, rejected);
    EXPECT_NE(reserved, producing);
    EXPECT_NE(reserved, confirmed);
    EXPECT_NE(reserved, release);
    EXPECT_NE(rejected, producing);
    EXPECT_NE(rejected, confirmed);
    EXPECT_NE(rejected, release);
    EXPECT_NE(producing, confirmed);
    EXPECT_NE(producing, release);
    EXPECT_NE(confirmed, release);
}

TEST(OrderStatusTest, EqualityComparisonWorks)
{
    OrderStatus a = OrderStatus::PRODUCING;
    OrderStatus b = OrderStatus::PRODUCING;

    EXPECT_EQ(a, b);
}
