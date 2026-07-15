#pragma once

#include <string>

namespace order_system {

// Sample entity.
// yield must be greater than 0 and less than or equal to 1, and stockQuantity must not be negative.
// Duplicate name validation is the Repository's responsibility and is not checked here.
class Sample {
public:
    Sample(int sampleId, const std::string& name, int avgProductionTime, double yield, int stockQuantity);

    int GetSampleId() const;
    const std::string& GetName() const;
    int GetAvgProductionTime() const;
    double GetYield() const;
    int GetStockQuantity() const;

private:
    int sampleId_;
    std::string name_;
    int avgProductionTime_;
    double yield_;
    int stockQuantity_;
};

}  // namespace order_system
