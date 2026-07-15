#pragma once

#include <string>

#include "../Model/SampleRepository.h"

namespace order_system {

// 시료 등록 요청 처리 결과.
struct SampleRegistrationResult {
    bool success;
    std::string errorMessage;
};

// SampleController: 입력 검증 후 SampleRepository 에 위임한다.
class SampleController {
public:
    explicit SampleController(ISampleRepository& repository);

    // 이름/평균 생산시간/수율을 검증한 뒤 유효하면 repository 에 등록을 위임한다.
    // 검증에 실패하면 등록을 취소하고 실패 사유를 결과에 담는다.
    SampleRegistrationResult RegisterSample(const std::string& name, int avgProductionTime, double yield);

private:
    ISampleRepository& repository_;
};

}  // namespace order_system
