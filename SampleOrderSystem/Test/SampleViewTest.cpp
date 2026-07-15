#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../View/SampleView.h"

using order_system::Sample;
using order_system::SampleRegistrationInput;
using order_system::SampleView;

namespace {

class SampleViewTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        originalCinBuffer_ = std::cin.rdbuf();
        originalCoutBuffer_ = std::cout.rdbuf();
    }

    void TearDown() override
    {
        std::cin.rdbuf(originalCinBuffer_);
        std::cout.rdbuf(originalCoutBuffer_);
    }

    std::streambuf* originalCinBuffer_ = nullptr;
    std::streambuf* originalCoutBuffer_ = nullptr;
};

}  // namespace

// 등록 입력폼: 이름/평균 생산시간/수율을 순서대로 입력받는다.
TEST_F(SampleViewTest, ReadSampleRegistrationInputReadsNameAvgTimeAndYield)
{
    std::istringstream input("SampleA\n30\n0.9\n");
    std::cin.rdbuf(input.rdbuf());

    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    SampleView view;
    SampleRegistrationInput result = view.ReadSampleRegistrationInput();

    EXPECT_EQ(result.name, "SampleA");
    EXPECT_EQ(result.avgProductionTime, 30);
    EXPECT_DOUBLE_EQ(result.yield, 0.9);
}

// 목록 출력: sampleId, name, avgProductionTime, yield, stockQuantity 컬럼이 모두 출력된다.
TEST_F(SampleViewTest, ShowSampleListPrintsAllColumns)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    std::vector<Sample> samples{ Sample(1, "SampleA", 30, 0.9, 5) };

    SampleView view;
    view.ShowSampleList(samples);

    const std::string text = output.str();
    EXPECT_NE(text.find("SampleA"), std::string::npos);
    EXPECT_NE(text.find("30"), std::string::npos);
    EXPECT_NE(text.find("0.9"), std::string::npos);
    EXPECT_NE(text.find("5"), std::string::npos);
}

// 등록된 시료가 없으면 안내 메시지를 출력한다.
TEST_F(SampleViewTest, ShowSampleListPrintsMessageWhenEmpty)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    SampleView view;
    view.ShowSampleList({});

    EXPECT_NE(output.str().find("등록된 시료가 없습니다"), std::string::npos);
}

// 검색 결과가 없으면 안내 메시지를 출력한다.
TEST_F(SampleViewTest, ShowSearchResultsPrintsMessageWhenEmpty)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    SampleView view;
    view.ShowSearchResults({});

    EXPECT_NE(output.str().find("검색 결과가 없습니다"), std::string::npos);
}

// 검색 결과 출력: 결과가 있으면 컬럼 정보가 표시된다.
TEST_F(SampleViewTest, ShowSearchResultsPrintsColumnsWhenNotEmpty)
{
    std::ostringstream output;
    std::cout.rdbuf(output.rdbuf());

    std::vector<Sample> samples{ Sample(2, "SampleB", 20, 0.8, 3) };

    SampleView view;
    view.ShowSearchResults(samples);

    const std::string text = output.str();
    EXPECT_NE(text.find("SampleB"), std::string::npos);
    EXPECT_NE(text.find("20"), std::string::npos);
    EXPECT_NE(text.find("0.8"), std::string::npos);
    EXPECT_NE(text.find("3"), std::string::npos);
}
