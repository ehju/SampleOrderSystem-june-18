#include "SampleController.h"

#include <algorithm>
#include <cctype>

namespace order_system {

namespace {

bool IsBlank(const std::string& text)
{
    return std::all_of(text.begin(), text.end(), [](unsigned char ch) { return std::isspace(ch); });
}

}  // namespace

SampleController::SampleController(ISampleRepository& repository)
    : repository_(repository)
{
}

SampleRegistrationResult SampleController::RegisterSample(const std::string& name, int avgProductionTime,
    double yield)
{
    if (IsBlank(name)) {
        return { false, "이름은 필수이며 공백일 수 없습니다." };
    }
    if (avgProductionTime <= 0) {
        return { false, "평균 생산시간은 양수여야 합니다." };
    }
    if (yield <= 0.0 || yield > 1.0) {
        return { false, "수율은 0 초과 1 이하여야 합니다." };
    }
    if (repository_.ExistsByName(name)) {
        return { false, "이미 존재하는 시료 이름입니다." };
    }

    repository_.Register(name, avgProductionTime, yield);
    return { true, "" };
}

}  // namespace order_system
