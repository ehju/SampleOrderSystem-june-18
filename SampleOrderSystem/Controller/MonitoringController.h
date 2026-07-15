#pragma once

#include <string>
#include <vector>

#include "../Model/OrderRepository.h"
#include "../Model/SampleRepository.h"

namespace order_system {

// 상태별 주문 개수. REJECTED 는 어떤 집계에도 포함하지 않는다.
struct OrderStatusCounts {
    int reservedCount = 0;
    int producingCount = 0;
    int confirmedCount = 0;
    int releaseCount = 0;
};

// 시료별 재고 상태 판정 결과.
enum class StockStatus {
    SUFFICIENT,  // 여유
    SHORTAGE,    // 부족
    DEPLETED     // 고갈
};

// 시료 하나에 대한 재고 현황 한 행.
struct SampleStockStatusRow {
    int sampleId;
    std::string name;
    int stockQuantity;
    int pendingQuantity;
    StockStatus status;
};

// MonitoringController: 상태별 주문 집계와 재고 상태 판정 유스케이스를 담당한다.
// 별도의 캐시 없이 항상 Repository 를 조회하여 실시간으로 계산한다.
class MonitoringController {
public:
    MonitoringController(ISampleRepository& sampleRepository, IOrderRepository& orderRepository);

    // RESERVED/PRODUCING/CONFIRMED/RELEASE 상태별 주문 개수를 계산한다. REJECTED 는 제외한다.
    OrderStatusCounts GetOrderStatusCounts() const;

    // 등록된 시료별로 stockQuantity 와 대기 주문 수량 합(RESERVED+PRODUCING+CONFIRMED)을 계산하고
    // 재고 상태(여유/부족/고갈)를 판정한다.
    std::vector<SampleStockStatusRow> GetStockStatuses() const;

private:
    ISampleRepository& sampleRepository_;
    IOrderRepository& orderRepository_;
};

}  // namespace order_system
