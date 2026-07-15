#include "ShipmentView.h"

#include <iostream>
#include <limits>

namespace order_system {

namespace {

int ReadIntOrInvalid(const std::string& prompt)
{
    std::cout << prompt;
    int value = 0;
    std::cin >> value;
    if (std::cin.fail()) {
        std::cin.clear();
        value = -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

}  // namespace

void ShipmentView::ShowMenu() const
{
    std::cout << "\n[출고 처리 메뉴]\n"
               << " 1. 출고 대상 조회\n"
               << " 2. 출고 처리\n"
               << " 0. 이전 메뉴로\n"
               << "선택 > ";
}

int ShipmentView::ReadMenuChoice() const
{
    return ReadIntOrInvalid("");
}

int ShipmentView::ReadOrderId(const std::string& prompt) const
{
    return ReadIntOrInvalid(prompt);
}

void ShipmentView::ShowShipmentTargets(const std::vector<Order>& orders) const
{
    if (orders.empty()) {
        std::cout << "출고 대상 주문이 없습니다\n";
        return;
    }
    for (const Order& order : orders) {
        std::cout << order.GetOrderId() << "\t" << order.GetSampleId() << "\t" << order.GetCustomerName() << "\t"
                   << order.GetQuantity() << "\n";
    }
}

void ShipmentView::ShowMessage(const std::string& message) const
{
    std::cout << message << "\n";
}

}  // namespace order_system
