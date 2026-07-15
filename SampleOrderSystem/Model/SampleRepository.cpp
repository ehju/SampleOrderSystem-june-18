#include "SampleRepository.h"

#include <algorithm>
#include <stdexcept>

#include "../Lib/Json.h"

namespace order_system {

SampleRepository::SampleRepository(std::filesystem::path jsonFilePath)
    : jsonFilePath_(std::move(jsonFilePath))
{
    LoadFromJsonFile();
}

void SampleRepository::LoadFromJsonFile()
{
    if (!std::filesystem::exists(jsonFilePath_)) {
        return;
    }

    json::JsonValue root = json::JsonValue::LoadFromFile(jsonFilePath_);
    for (const json::JsonValue& item : root.AsArray()) {
        int sampleId = static_cast<int>(item.Find("sampleId")->AsNumber());
        std::string name = item.Find("name")->AsString();
        int avgProductionTime = static_cast<int>(item.Find("avgProductionTime")->AsNumber());
        double yield = item.Find("yield")->AsNumber();
        int stockQuantity = static_cast<int>(item.Find("stockQuantity")->AsNumber());

        samples_.emplace_back(sampleId, name, avgProductionTime, yield, stockQuantity);
        nextSampleId_ = std::max(nextSampleId_, sampleId + 1);
    }
}

void SampleRepository::SaveToJsonFile() const
{
    json::JsonValue root = json::JsonValue::MakeArray();
    for (const Sample& sample : samples_) {
        json::JsonValue item = json::JsonValue::MakeObject();
        item["sampleId"] = sample.GetSampleId();
        item["name"] = sample.GetName();
        item["avgProductionTime"] = sample.GetAvgProductionTime();
        item["yield"] = sample.GetYield();
        item["stockQuantity"] = sample.GetStockQuantity();
        root.Push(std::move(item));
    }

    std::filesystem::path parentDir = jsonFilePath_.parent_path();
    if (!parentDir.empty()) {
        std::filesystem::create_directories(parentDir);
    }
    root.SaveToFile(jsonFilePath_);
}

Sample SampleRepository::Register(const std::string& name, int avgProductionTime, double yield)
{
    if (ExistsByName(name)) {
        throw std::invalid_argument("이미 존재하는 시료 이름입니다.");
    }

    Sample sample(nextSampleId_, name, avgProductionTime, yield, 0);
    ++nextSampleId_;
    samples_.push_back(sample);
    SaveToJsonFile();

    return sample;
}

std::vector<Sample> SampleRepository::GetAll() const
{
    return samples_;
}

std::vector<Sample> SampleRepository::SearchByName(const std::string& keyword) const
{
    std::vector<Sample> result;
    for (const Sample& sample : samples_) {
        if (sample.GetName().find(keyword) != std::string::npos) {
            result.push_back(sample);
        }
    }
    return result;
}

std::optional<Sample> SampleRepository::FindById(int sampleId) const
{
    for (const Sample& sample : samples_) {
        if (sample.GetSampleId() == sampleId) {
            return sample;
        }
    }
    return std::nullopt;
}

bool SampleRepository::ExistsByName(const std::string& name) const
{
    for (const Sample& sample : samples_) {
        if (sample.GetName() == name) {
            return true;
        }
    }
    return false;
}

void SampleRepository::IncreaseStock(int sampleId, int quantity)
{
    for (Sample& sample : samples_) {
        if (sample.GetSampleId() == sampleId) {
            sample = Sample(sample.GetSampleId(), sample.GetName(), sample.GetAvgProductionTime(),
                sample.GetYield(), sample.GetStockQuantity() + quantity);
            SaveToJsonFile();
            return;
        }
    }
}

void SampleRepository::DecreaseStock(int sampleId, int quantity)
{
    for (Sample& sample : samples_) {
        if (sample.GetSampleId() == sampleId) {
            sample = Sample(sample.GetSampleId(), sample.GetName(), sample.GetAvgProductionTime(),
                sample.GetYield(), sample.GetStockQuantity() - quantity);
            SaveToJsonFile();
            return;
        }
    }
}

}  // namespace order_system
