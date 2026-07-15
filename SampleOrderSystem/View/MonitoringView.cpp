#include "MonitoringView.h"

#include <iostream>

namespace order_system {

namespace {

std::string ToDisplayText(StockStatus status)
{
    switch (status) {
        case StockStatus::SUFFICIENT:
            return "여유";
        case StockStatus::SHORTAGE:
            return "부족";
        case StockStatus::DEPLETED:
            return "고갈";
    }
    return "";
}

}  // namespace

void MonitoringView::ShowOrderStatus(const OrderStatusCounts& counts) const
{
    std::cout << "[주문 현황]\n";
    std::cout << " RESERVED  : " << counts.reservedCount << "건\n";
    std::cout << " PRODUCING : " << counts.producingCount << "건\n";
    std::cout << " CONFIRMED : " << counts.confirmedCount << "건\n";
    std::cout << " RELEASE   : " << counts.releaseCount << "건\n";
}

void MonitoringView::ShowStockStatus(const std::vector<SampleStockStatusRow>& rows) const
{
    if (rows.empty()) {
        std::cout << "등록된 시료가 없습니다\n";
        return;
    }

    std::cout << "[재고 현황]\n";
    std::cout << " sampleId | name     | stockQuantity | 대기수량합 | 상태\n";
    for (const SampleStockStatusRow& row : rows) {
        std::cout << "    " << row.sampleId << "     | " << row.name << "   |      " << row.stockQuantity
                   << "       |     " << row.pendingQuantity << "     | " << ToDisplayText(row.status) << "\n";
    }
}

}  // namespace order_system
