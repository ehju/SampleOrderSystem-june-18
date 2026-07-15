#include "ProductionView.h"

#include <iostream>
#include <limits>

namespace order_system {

void ProductionView::ShowMenu() const
{
    std::cout << "\n[생산 라인 메뉴]\n"
               << " 1. 생산 대기열 조회\n"
               << " 2. 생산 완료 처리\n"
               << " 0. 이전 메뉴로\n"
               << "선택 > ";
}

int ProductionView::ReadMenuChoice() const
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

void ProductionView::ShowQueue(const std::vector<ProductionJob>& jobs) const
{
    if (jobs.empty()) {
        std::cout << "대기 중인 생산 작업이 없습니다\n";
        return;
    }

    int sequence = 1;
    for (const ProductionJob& job : jobs) {
        std::cout << sequence << "\t" << job.GetOrderId() << "\t" << job.GetSampleId() << "\t"
                   << job.GetShortageQuantity() << "\t" << job.GetActualProductionQuantity() << "\t"
                   << job.GetTotalProductionTime() << "\n";
        ++sequence;
    }
}

void ProductionView::ShowCompletedJob(const ProductionJob& job) const
{
    std::cout << "생산 완료 처리되었습니다. orderId: " << job.GetOrderId() << ", sampleId: " << job.GetSampleId()
               << ", 생산량: " << job.GetActualProductionQuantity() << "\n";
}

void ProductionView::ShowMessage(const std::string& message) const
{
    std::cout << message << "\n";
}

}  // namespace order_system
