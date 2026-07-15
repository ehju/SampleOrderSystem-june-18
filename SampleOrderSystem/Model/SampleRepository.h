#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Sample.h"

namespace order_system {

// SampleRepository 의 추상 인터페이스.
// SampleController 단위 테스트에서 GoogleMock 으로 대체하기 위해 사용한다.
class ISampleRepository {
public:
    virtual ~ISampleRepository() = default;

    // 새 Sample 을 등록한다. sampleId 는 자동 채번하고 stockQuantity 는 0으로 초기화한다.
    // 이름이 이미 등록되어 있으면 std::invalid_argument 를 던진다.
    virtual Sample Register(const std::string& name, int avgProductionTime, double yield) = 0;

    // 등록된 모든 Sample 목록을 반환한다.
    virtual std::vector<Sample> GetAll() const = 0;

    // 이름에 keyword 가 포함된 Sample 목록을 반환한다.
    virtual std::vector<Sample> SearchByName(const std::string& keyword) const = 0;

    // sampleId 로 단건 조회한다. 존재하지 않으면 std::nullopt 를 반환한다.
    virtual std::optional<Sample> FindById(int sampleId) const = 0;

    // 이름이 이미 등록되어 있는지 확인한다.
    virtual bool ExistsByName(const std::string& name) const = 0;

    // sampleId 에 해당하는 stockQuantity 를 증가시킨다.
    virtual void IncreaseStock(int sampleId, int quantity) = 0;

    // sampleId 에 해당하는 stockQuantity 를 감소시킨다.
    virtual void DecreaseStock(int sampleId, int quantity) = 0;
};

// JSON 파일 기반으로 Sample 을 영속화하는 Repository 구현체.
// 생성 시 jsonFilePath 가 존재하면 그 내용을 읽어 초기 상태를 구성하고,
// 등록/재고 갱신 등 변경이 있을 때마다 즉시 JSON 파일에 반영한다.
class SampleRepository : public ISampleRepository {
public:
    explicit SampleRepository(std::filesystem::path jsonFilePath);

    Sample Register(const std::string& name, int avgProductionTime, double yield) override;
    std::vector<Sample> GetAll() const override;
    std::vector<Sample> SearchByName(const std::string& keyword) const override;
    std::optional<Sample> FindById(int sampleId) const override;
    bool ExistsByName(const std::string& name) const override;
    void IncreaseStock(int sampleId, int quantity) override;
    void DecreaseStock(int sampleId, int quantity) override;

private:
    // jsonFilePath_ 가 존재하면 그 내용을 읽어 samples_/nextSampleId_ 를 구성한다.
    void LoadFromJsonFile();

    // samples_ 의 현재 상태를 jsonFilePath_ 에 즉시 반영한다.
    void SaveToJsonFile() const;

    std::filesystem::path jsonFilePath_;
    std::vector<Sample> samples_;
    int nextSampleId_ = 1;
};

}  // namespace order_system
