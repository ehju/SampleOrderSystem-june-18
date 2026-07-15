#include "MonitoringView.h"

#include <iostream>
#include <limits>

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

void MonitoringView::ShowMenu() const
{
    std::cout << "\n[모니터링 메뉴]\n"
               << " 1. 주문 현황 조회\n"
               << " 2. 재고 현황 조회\n"
               << " 0. 이전 메뉴로\n"
               << "선택 > ";
}

int MonitoringView::ReadMenuChoice() const
{
    int choice = 0;
    std::cin >> choice;
    if (std::cin.fail()) {
        std::cin.clear();
        choice = -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

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

void MonitoringView::ShowMessage(const std::string& message) const
{
    std::cout << message << "\n";
}

}  // namespace order_system
