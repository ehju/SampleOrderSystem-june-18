#include "Sample.h"

#include <stdexcept>

namespace order_system {

Sample::Sample(int sampleId, const std::string& name, int avgProductionTime, double yield, int stockQuantity)
    : sampleId_(sampleId),
      name_(name),
      avgProductionTime_(avgProductionTime),
      yield_(yield),
      stockQuantity_(stockQuantity)
{
    if (yield <= 0.0 || yield > 1.0) {
        throw std::invalid_argument("yield must be greater than 0 and less than or equal to 1");
    }
    if (stockQuantity < 0) {
        throw std::invalid_argument("stockQuantity must not be negative");
    }
}

int Sample::GetSampleId() const
{
    return sampleId_;
}

const std::string& Sample::GetName() const
{
    return name_;
}

int Sample::GetAvgProductionTime() const
{
    return avgProductionTime_;
}

double Sample::GetYield() const
{
    return yield_;
}

int Sample::GetStockQuantity() const
{
    return stockQuantity_;
}

}  // namespace order_system
