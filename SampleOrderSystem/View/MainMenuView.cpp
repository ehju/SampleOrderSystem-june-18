#include "MainMenuView.h"

#include <iostream>
#include <stdexcept>

namespace order_system {

void MainMenuView::ShowMainMenu(const MainMenuSummary& summary) const
{
    std::cout << "==================================================\n"
               << " 반도체 시료 생산주문관리 시스템\n"
               << "==================================================\n"
               << "[요약]\n"
               << " 등록 시료 수 : " << summary.registeredSampleCount << "\n"
               << " 총 재고 수량 : " << summary.totalStockQuantity << "\n"
               << " 진행 중 주문 : RESERVED " << summary.orderStatusCounts.reservedCount << " / PRODUCING "
               << summary.orderStatusCounts.producingCount << " / CONFIRMED "
               << summary.orderStatusCounts.confirmedCount << "\n"
               << "\n"
               << "[메뉴]\n"
               << " 1. 시료 관리\n"
               << " 2. 주문 (접수 / 승인 / 거절)\n"
               << " 3. 모니터링\n"
               << " 4. 출고 처리\n"
               << " 5. 생산 라인\n"
               << " 0. 종료\n"
               << "--------------------------------------------------\n"
               << "선택 > ";
}

int MainMenuView::ReadMenuChoice() const
{
    std::string line;
    std::getline(std::cin, line);

    try {
        size_t consumed = 0;
        int value = std::stoi(line, &consumed);
        if (consumed != line.size()) {
            return -1;
        }
        return value;
    } catch (const std::exception&) {
        return -1;
    }
}

void MainMenuView::ShowMessage(const std::string& message) const
{
    std::cout << message << "\n";
}

}  // namespace order_system
