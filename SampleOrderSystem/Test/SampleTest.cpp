#include <gtest/gtest.h>
#include <stdexcept>

#include "../Model/Sample.h"

using order_system::Sample;

// 정상 값으로 생성 시 getter 가 입력값을 그대로 반환한다.
TEST(SampleTest, ConstructsWithValidValuesAndGettersReturnThem)
{
    Sample sample(1, "SampleA", 30, 0.9, 100);

    EXPECT_EQ(sample.GetSampleId(), 1);
    EXPECT_EQ(sample.GetName(), "SampleA");
    EXPECT_EQ(sample.GetAvgProductionTime(), 30);
    EXPECT_DOUBLE_EQ(sample.GetYield(), 0.9);
    EXPECT_EQ(sample.GetStockQuantity(), 100);
}

// yield 는 0을 초과해야 한다. 0 이하이면 생성이 거부된다.
TEST(SampleTest, RejectsYieldOfZero)
{
    EXPECT_THROW(Sample(1, "SampleA", 30, 0.0, 100), std::invalid_argument);
}

TEST(SampleTest, RejectsNegativeYield)
{
    EXPECT_THROW(Sample(1, "SampleA", 30, -0.1, 100), std::invalid_argument);
}

// yield 는 1을 초과할 수 없다.
TEST(SampleTest, RejectsYieldGreaterThanOne)
{
    EXPECT_THROW(Sample(1, "SampleA", 30, 1.1, 100), std::invalid_argument);
}

// yield 가 정확히 1.0 인 경우는 허용된다 (0 초과 1 이하).
TEST(SampleTest, AcceptsYieldOfExactlyOne)
{
    EXPECT_NO_THROW(Sample(1, "SampleA", 30, 1.0, 100));
}

// stockQuantity 는 음수가 될 수 없다.
TEST(SampleTest, RejectsNegativeStockQuantity)
{
    EXPECT_THROW(Sample(1, "SampleA", 30, 0.9, -1), std::invalid_argument);
}

// stockQuantity 가 0 인 경우는 허용된다 (0 이상).
TEST(SampleTest, AcceptsZeroStockQuantity)
{
    EXPECT_NO_THROW(Sample(1, "SampleA", 30, 0.9, 0));
}

// 이름 중복 검증은 Repository 의 책임이므로 Sample 생성자는 이를 검증하지 않는다.
// (Phase 2에서 Repository 를 통해 다룰 범위이며, 이 phase 에서는 동일한 이름으로
//  여러 Sample 인스턴스를 생성해도 예외가 발생하지 않는지만 확인한다.)
TEST(SampleTest, DoesNotValidateDuplicateNameAtConstructionLevel)
{
    EXPECT_NO_THROW(Sample(1, "DuplicateName", 30, 0.9, 100));
    EXPECT_NO_THROW(Sample(2, "DuplicateName", 30, 0.9, 100));
}
