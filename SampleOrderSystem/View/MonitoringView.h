#pragma once

#include <string>
#include <vector>

#include "../Controller/MonitoringController.h"

namespace order_system {

// MonitoringView: 주문 현황/재고 현황 화면 출력만 담당한다.
class MonitoringView {
public:
    void ShowMenu() const;

    // 메뉴 선택 값을 입력받는다. 숫자가 아니면 -1을 반환한다.
    int ReadMenuChoice() const;

    // 상태별 주문 개수를 출력한다.
    void ShowOrderStatus(const OrderStatusCounts& counts) const;

    // 시료별 재고 현황을 출력한다. rows 가 비어있으면 "등록된 시료가 없습니다" 를 출력한다.
    void ShowStockStatus(const std::vector<SampleStockStatusRow>& rows) const;

    void ShowMessage(const std::string& message) const;
};

}  // namespace order_system
