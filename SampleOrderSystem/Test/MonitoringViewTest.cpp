#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../Controller/MonitoringController.h"
#include "../View/MonitoringView.h"

using order_system::MonitoringView;
using order_system::OrderStatusCounts;
using order_system::SampleStockStatusRow;
using order_system::StockStatus;

namespace {

class MonitoringViewTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        originalCoutBuffer_ = std::cout.rdbuf();
    }

    void TearDown() override
    {
        std::cout.rdbuf(originalCoutBuffer_);
    }

    std::streambuf* originalCoutBuffer_ = nullptr;
};

}  // namespace

// 주문 현황 출력: RESERVED/PRODUCING/CONFIRMED/RELEASE 각 상태와 건수가 모두 표시된다.
TEST_F(MonitoringViewTest, ShowOrderStatusPrintsAllStatusCounts)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    OrderStatusCounts counts;
    counts.reservedCount = 3;
    counts.producingCount = 2;
    counts.confirmedCount = 4;
    counts.releaseCount = 10;

    MonitoringView view;
    view.ShowOrderStatus(counts);

    const std::string text = output.str();
    EXPECT_NE(text.find("RESERVED"), std::string::npos);
    EXPECT_NE(text.find("3"), std::string::npos);
    EXPECT_NE(text.find("PRODUCING"), std::string::npos);
    EXPECT_NE(text.find("2"), std::string::npos);
    EXPECT_NE(text.find("CONFIRMED"), std::string::npos);
    EXPECT_NE(text.find("4"), std::string::npos);
    EXPECT_NE(text.find("RELEASE"), std::string::npos);
    EXPECT_NE(text.find("10"), std::string::npos);
}

// 주문이 하나도 없는 경우: 모든 상태 카운트가 0으로 표시된다.
TEST_F(MonitoringViewTest, ShowOrderStatusPrintsZeroCountsWhenNoOrders)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    OrderStatusCounts counts;

    MonitoringView view;
    view.ShowOrderStatus(counts);

    const std::string text = output.str();
    EXPECT_NE(text.find("RESERVED"), std::string::npos);
    EXPECT_NE(text.find("PRODUCING"), std::string::npos);
    EXPECT_NE(text.find("CONFIRMED"), std::string::npos);
    EXPECT_NE(text.find("RELEASE"), std::string::npos);
    EXPECT_NE(text.find("0"), std::string::npos);
}

// 재고 현황 출력: sampleId/name/stockQuantity/대기수량합/상태(여유) 가 모두 표시된다.
TEST_F(MonitoringViewTest, ShowStockStatusPrintsSufficientRow)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    std::vector<SampleStockStatusRow> rows{
        SampleStockStatusRow{ 1, "WaferA", 50, 30, StockStatus::SUFFICIENT },
    };

    MonitoringView view;
    view.ShowStockStatus(rows);

    const std::string text = output.str();
    EXPECT_NE(text.find("WaferA"), std::string::npos);
    EXPECT_NE(text.find("50"), std::string::npos);
    EXPECT_NE(text.find("30"), std::string::npos);
    EXPECT_NE(text.find("여유"), std::string::npos);
}

// 재고 현황 출력: 상태가 "고갈" 인 행이 올바르게 표시된다.
TEST_F(MonitoringViewTest, ShowStockStatusPrintsDepletedRow)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    std::vector<SampleStockStatusRow> rows{
        SampleStockStatusRow{ 2, "WaferB", 0, 10, StockStatus::DEPLETED },
    };

    MonitoringView view;
    view.ShowStockStatus(rows);

    EXPECT_NE(output.str().find("고갈"), std::string::npos);
}

// 재고 현황 출력: 상태가 "부족" 인 행이 올바르게 표시된다.
TEST_F(MonitoringViewTest, ShowStockStatusPrintsShortageRow)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    std::vector<SampleStockStatusRow> rows{
        SampleStockStatusRow{ 3, "WaferC", 5, 20, StockStatus::SHORTAGE },
    };

    MonitoringView view;
    view.ShowStockStatus(rows);

    EXPECT_NE(output.str().find("부족"), std::string::npos);
}

// 시료가 하나도 없는 경우: 재고 현황 목록이 비어 있다는 안내를 출력한다.
TEST_F(MonitoringViewTest, ShowStockStatusPrintsMessageWhenEmpty)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    MonitoringView view;
    view.ShowStockStatus({});

    EXPECT_NE(output.str().find("등록된 시료가 없습니다"), std::string::npos);
}
