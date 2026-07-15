#pragma once

#include <string>
#include <vector>

#include "../Model/Sample.h"

namespace order_system {

// 시료 등록 입력폼에서 받은 값.
struct SampleRegistrationInput {
    std::string name;
    int avgProductionTime;
    double yield;
};

// SampleView: 시료 등록/조회/검색과 관련된 화면 출력과 입력 수신만 담당한다.
class SampleView {
public:
    // 이름/평균 생산시간/수율을 순서대로 입력받는다.
    SampleRegistrationInput ReadSampleRegistrationInput() const;

    // 이름 부분검색을 위한 검색어를 입력받는다.
    std::string ReadSearchKeyword() const;

    // 전체 조회 결과를 sampleId, name, avgProductionTime, yield, stockQuantity 컬럼으로 출력한다.
    // samples 가 비어있으면 "등록된 시료가 없습니다" 를 출력한다.
    void ShowSampleList(const std::vector<Sample>& samples) const;

    // 검색 결과를 sampleId, name, avgProductionTime, yield, stockQuantity 컬럼으로 출력한다.
    // samples 가 비어있으면 "검색 결과가 없습니다" 를 출력한다.
    void ShowSearchResults(const std::vector<Sample>& samples) const;

    void ShowMessage(const std::string& message) const;
};

}  // namespace order_system
