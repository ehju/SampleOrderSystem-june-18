#pragma once

#include <chrono>

namespace order_system {

// ProductionJob entity enqueued into the production queue.
// This phase only provides the constructor/getters; computing actualProductionQuantity /
// totalProductionTime (e.g. ceil calculation) is the responsibility of the approval/production
// logic in Phase 4/5.
class ProductionJob {
public:
    ProductionJob(int orderId, int sampleId, int shortageQuantity, int actualProductionQuantity,
                  double totalProductionTime, const std::chrono::system_clock::time_point& enqueuedAt,
                  int producedQuantity);

    int GetOrderId() const;
    int GetSampleId() const;
    int GetShortageQuantity() const;
    int GetActualProductionQuantity() const;
    double GetTotalProductionTime() const;
    const std::chrono::system_clock::time_point& GetEnqueuedAt() const;
    int GetProducedQuantity() const;

private:
    int orderId_;
    int sampleId_;
    int shortageQuantity_;
    int actualProductionQuantity_;
    double totalProductionTime_;
    std::chrono::system_clock::time_point enqueuedAt_;
    int producedQuantity_;
};

}  // namespace order_system
