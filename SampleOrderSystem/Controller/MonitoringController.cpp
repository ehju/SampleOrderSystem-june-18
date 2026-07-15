#include "MonitoringController.h"

namespace order_system {

namespace {

// stockQuantity 와 대기 수량 합을 비교해 재고 상태를 판정한다.
// 고갈 조건(stockQuantity == 0)이 부족 조건보다 우선한다.
StockStatus JudgeStockStatus(int stockQuantity, int pendingQuantity)
{
    if (stockQuantity == 0) {
        return StockStatus::DEPLETED;
    }
    if (stockQuantity < pendingQuantity) {
        return StockStatus::SHORTAGE;
    }
    return StockStatus::SUFFICIENT;
}

}  // namespace

MonitoringController::MonitoringController(ISampleRepository& sampleRepository, IOrderRepository& orderRepository)
    : sampleRepository_(sampleRepository),
      orderRepository_(orderRepository)
{
}

OrderStatusCounts MonitoringController::GetOrderStatusCounts() const
{
    OrderStatusCounts counts;
    for (const Order& order : orderRepository_.GetAll()) {
        switch (order.GetStatus()) {
            case OrderStatus::RESERVED:
                ++counts.reservedCount;
                break;
            case OrderStatus::PRODUCING:
                ++counts.producingCount;
                break;
            case OrderStatus::CONFIRMED:
                ++counts.confirmedCount;
                break;
            case OrderStatus::RELEASE:
                ++counts.releaseCount;
                break;
            case OrderStatus::REJECTED:
                break;
        }
    }
    return counts;
}

std::vector<SampleStockStatusRow> MonitoringController::GetStockStatuses() const
{
    std::vector<Sample> samples = sampleRepository_.GetAll();
    std::vector<Order> orders = orderRepository_.GetAll();

    std::vector<SampleStockStatusRow> rows;
    rows.reserve(samples.size());
    for (const Sample& sample : samples) {
        int pendingQuantity = 0;
        for (const Order& order : orders) {
            if (order.GetSampleId() != sample.GetSampleId()) {
                continue;
            }
            OrderStatus status = order.GetStatus();
            if (status == OrderStatus::RESERVED || status == OrderStatus::PRODUCING
                || status == OrderStatus::CONFIRMED) {
                pendingQuantity += order.GetQuantity();
            }
        }

        SampleStockStatusRow row;
        row.sampleId = sample.GetSampleId();
        row.name = sample.GetName();
        row.stockQuantity = sample.GetStockQuantity();
        row.pendingQuantity = pendingQuantity;
        row.status = JudgeStockStatus(row.stockQuantity, row.pendingQuantity);
        rows.push_back(row);
    }
    return rows;
}

}  // namespace order_system
