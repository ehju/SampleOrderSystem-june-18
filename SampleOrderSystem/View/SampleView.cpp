#include "SampleView.h"

#include <iostream>

namespace order_system {

namespace {

void PrintSampleTable(const std::vector<Sample>& samples)
{
    for (const Sample& sample : samples) {
        std::cout << sample.GetSampleId() << "\t" << sample.GetName() << "\t" << sample.GetAvgProductionTime()
                   << "\t" << sample.GetYield() << "\t" << sample.GetStockQuantity() << "\n";
    }
}

}  // namespace

SampleRegistrationInput SampleView::ReadSampleRegistrationInput() const
{
    SampleRegistrationInput input;

    std::cout << "이름: ";
    std::getline(std::cin, input.name);

    std::cout << "평균 생산시간: ";
    std::cin >> input.avgProductionTime;

    std::cout << "수율: ";
    std::cin >> input.yield;
    std::cin.ignore();

    return input;
}

std::string SampleView::ReadSearchKeyword() const
{
    std::string keyword;
    std::cout << "검색어: ";
    std::getline(std::cin, keyword);
    return keyword;
}

void SampleView::ShowSampleList(const std::vector<Sample>& samples) const
{
    if (samples.empty()) {
        std::cout << "등록된 시료가 없습니다\n";
        return;
    }
    PrintSampleTable(samples);
}

void SampleView::ShowSearchResults(const std::vector<Sample>& samples) const
{
    if (samples.empty()) {
        std::cout << "검색 결과가 없습니다\n";
        return;
    }
    PrintSampleTable(samples);
}

void SampleView::ShowMessage(const std::string& message) const
{
    std::cout << message << "\n";
}

}  // namespace order_system
