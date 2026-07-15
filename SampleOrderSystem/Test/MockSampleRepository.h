#pragma once

#include <gmock/gmock.h>

#include "../Model/SampleRepository.h"

namespace order_system {

// SampleController 단위 테스트에서 SampleRepository 를 대체하기 위한 GoogleMock 목 객체.
class MockSampleRepository : public ISampleRepository {
public:
    MOCK_METHOD(Sample, Register, (const std::string& name, int avgProductionTime, double yield), (override));
    MOCK_METHOD(std::vector<Sample>, GetAll, (), (const, override));
    MOCK_METHOD(std::vector<Sample>, SearchByName, (const std::string& keyword), (const, override));
    MOCK_METHOD(std::optional<Sample>, FindById, (int sampleId), (const, override));
    MOCK_METHOD(bool, ExistsByName, (const std::string& name), (const, override));
    MOCK_METHOD(void, IncreaseStock, (int sampleId, int quantity), (override));
    MOCK_METHOD(void, DecreaseStock, (int sampleId, int quantity), (override));
};

}  // namespace order_system
