#include <gtest/gtest.h>

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include "../Controller/MainMenuController.h"
#include "../Controller/MonitoringController.h"
#include "../Controller/OrderController.h"
#include "../Controller/ProductionController.h"
#include "../Controller/SampleController.h"
#include "../Controller/ShipmentController.h"
#include "../Model/OrderRepository.h"
#include "../Model/ProductionQueue.h"
#include "../Model/SampleRepository.h"

using order_system::MainMenuController;
using order_system::MonitoringController;
using order_system::OrderController;
using order_system::OrderRepository;
using order_system::ProductionController;
using order_system::ProductionQueue;
using order_system::SampleController;
using order_system::SampleRepository;
using order_system::ShipmentController;

namespace {

namespace fs = std::filesystem;

// 메인 메뉴 통합 테스트: 실제 Repository/Controller 를 엮어 입력 처리 경계값을 검증한다.
class MainMenuControllerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        testDir_ = fs::temp_directory_path() / "SampleOrderSystemTest" /
            ::testing::UnitTest::GetInstance()->current_test_info()->name();

        std::error_code ec;
        fs::remove_all(testDir_, ec);
        fs::create_directories(testDir_, ec);

        originalCinBuffer_ = std::cin.rdbuf();
        originalCoutBuffer_ = std::cout.rdbuf();
    }

    void TearDown() override
    {
        std::cin.rdbuf(originalCinBuffer_);
        std::cout.rdbuf(originalCoutBuffer_);

        std::error_code ec;
        fs::remove_all(testDir_, ec);
    }

    // scriptedInput 을 표준 입력으로 주입하고 MainMenuController::Run() 을 실행한 뒤 표준 출력을 반환한다.
    std::string RunMainMenu(const std::string& scriptedInput)
    {
        std::istringstream input(scriptedInput);
        std::ostringstream output;
        std::cin.rdbuf(input.rdbuf());
        std::cout.rdbuf(output.rdbuf());

        SampleRepository sampleRepository(testDir_ / "samples.json");
        OrderRepository orderRepository(testDir_ / "orders.json");
        ProductionQueue productionQueue;

        SampleController sampleController(sampleRepository);
        OrderController orderController(sampleRepository, orderRepository);
        MonitoringController monitoringController(sampleRepository, orderRepository);
        ProductionController productionController(productionQueue, sampleRepository, orderRepository);
        ShipmentController shipmentController(sampleRepository, orderRepository);

        MainMenuController mainMenuController(sampleRepository, sampleController, orderController,
                                               monitoringController, productionController, shipmentController,
                                               productionQueue);
        mainMenuController.Run();

        return output.str();
    }

    fs::path testDir_;
    std::streambuf* originalCinBuffer_ = nullptr;
    std::streambuf* originalCoutBuffer_ = nullptr;
};

}  // namespace

// 초기 상태(등록된 시료 0건)에서 요약 화면이 0으로 표시된다.
TEST_F(MainMenuControllerTest, ShowsZeroSummaryWhenNoSamplesRegistered)
{
    std::string output = RunMainMenu("0\n");

    EXPECT_NE(output.find("등록 시료 수 : 0"), std::string::npos);
    EXPECT_NE(output.find("총 재고 수량 : 0"), std::string::npos);
}

// 숫자가 아닌 입력은 잘못된 입력으로 처리되고 메뉴 루프가 계속된다.
TEST_F(MainMenuControllerTest, TreatsNonNumericInputAsInvalidAndContinuesLoop)
{
    std::string output = RunMainMenu("abc\n0\n");

    EXPECT_NE(output.find("잘못된 입력입니다"), std::string::npos);
}

// 메뉴 범위(0~5)를 벗어난 선택은 잘못된 입력으로 처리되고 메뉴 루프가 계속된다.
TEST_F(MainMenuControllerTest, TreatsOutOfRangeChoiceAsInvalidAndContinuesLoop)
{
    std::string output = RunMainMenu("99\n0\n");

    EXPECT_NE(output.find("잘못된 입력입니다"), std::string::npos);
}
