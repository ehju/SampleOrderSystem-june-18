#pragma once

#include <vector>

#include "../Controller/MonitoringController.h"

namespace order_system {

// MonitoringView: 주문 현황/재고 현황 화면 출력만 담당한다.
class MonitoringView {
public:
    // 상태별 주문 개수를 출력한다.
    void ShowOrderStatus(const OrderStatusCounts& counts) const;

    // 시료별 재고 현황을 출력한다. rows 가 비어있으면 "등록된 시료가 없습니다" 를 출력한다.
    void ShowStockStatus(const std::vector<SampleStockStatusRow>& rows) const;
};

}  // namespace order_system
