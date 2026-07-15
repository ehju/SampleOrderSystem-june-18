#include "ProductionJob.h"

namespace order_system {

ProductionJob::ProductionJob(int orderId, int sampleId, int shortageQuantity, int actualProductionQuantity,
                              double totalProductionTime, const std::chrono::system_clock::time_point& enqueuedAt,
                              int producedQuantity)
    : orderId_(orderId),
      sampleId_(sampleId),
      shortageQuantity_(shortageQuantity),
      actualProductionQuantity_(actualProductionQuantity),
      totalProductionTime_(totalProductionTime),
      enqueuedAt_(enqueuedAt),
      producedQuantity_(producedQuantity)
{
}

int ProductionJob::GetOrderId() const
{
    return orderId_;
}

int ProductionJob::GetSampleId() const
{
    return sampleId_;
}

int ProductionJob::GetShortageQuantity() const
{
    return shortageQuantity_;
}

int ProductionJob::GetActualProductionQuantity() const
{
    return actualProductionQuantity_;
}

double ProductionJob::GetTotalProductionTime() const
{
    return totalProductionTime_;
}

const std::chrono::system_clock::time_point& ProductionJob::GetEnqueuedAt() const
{
    return enqueuedAt_;
}

int ProductionJob::GetProducedQuantity() const
{
    return producedQuantity_;
}

}  // namespace order_system
