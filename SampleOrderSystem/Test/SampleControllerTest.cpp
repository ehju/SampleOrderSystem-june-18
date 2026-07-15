#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Controller/SampleController.h"
#include "MockSampleRepository.h"

using ::testing::_;
using ::testing::Return;
using order_system::MockSampleRepository;
using order_system::Sample;
using order_system::SampleController;
using order_system::SampleRegistrationResult;

// 빈 이름은 등록을 거부한다.
TEST(SampleControllerTest, RejectsEmptyName)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, Register(_, _, _)).Times(0);

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("", 30, 0.9);

    EXPECT_FALSE(result.success);
}

// 공백만으로 구성된 이름도 등록을 거부한다.
TEST(SampleControllerTest, RejectsBlankName)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, Register(_, _, _)).Times(0);

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("   ", 30, 0.9);

    EXPECT_FALSE(result.success);
}

// 이미 등록된 이름이면 등록을 거부한다.
TEST(SampleControllerTest, RejectsDuplicateName)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, ExistsByName("SampleA")).WillOnce(Return(true));
    EXPECT_CALL(mockRepository, Register(_, _, _)).Times(0);

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("SampleA", 30, 0.9);

    EXPECT_FALSE(result.success);
}

// 수율이 0이면 등록을 거부한다.
TEST(SampleControllerTest, RejectsYieldOfZero)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, Register(_, _, _)).Times(0);

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("SampleA", 30, 0.0);

    EXPECT_FALSE(result.success);
}

// 수율이 음수이면 등록을 거부한다.
TEST(SampleControllerTest, RejectsNegativeYield)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, Register(_, _, _)).Times(0);

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("SampleA", 30, -0.1);

    EXPECT_FALSE(result.success);
}

// 수율이 1을 초과하면 등록을 거부한다.
TEST(SampleControllerTest, RejectsYieldGreaterThanOne)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, Register(_, _, _)).Times(0);

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("SampleA", 30, 1.1);

    EXPECT_FALSE(result.success);
}

// 평균 생산시간이 0이면 등록을 거부한다.
TEST(SampleControllerTest, RejectsAvgProductionTimeOfZero)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, Register(_, _, _)).Times(0);

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("SampleA", 0, 0.9);

    EXPECT_FALSE(result.success);
}

// 평균 생산시간이 음수이면 등록을 거부한다.
TEST(SampleControllerTest, RejectsNegativeAvgProductionTime)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, Register(_, _, _)).Times(0);

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("SampleA", -1, 0.9);

    EXPECT_FALSE(result.success);
}

// 정상 입력이면 Repository 의 Register 메서드가 호출되고 성공 결과를 반환한다.
TEST(SampleControllerTest, RegistersSampleWhenInputIsValid)
{
    MockSampleRepository mockRepository;
    EXPECT_CALL(mockRepository, ExistsByName("SampleA")).WillOnce(Return(false));
    EXPECT_CALL(mockRepository, Register("SampleA", 30, 0.9))
        .WillOnce(Return(Sample(1, "SampleA", 30, 0.9, 0)));

    SampleController controller(mockRepository);
    SampleRegistrationResult result = controller.RegisterSample("SampleA", 30, 0.9);

    EXPECT_TRUE(result.success);
}
