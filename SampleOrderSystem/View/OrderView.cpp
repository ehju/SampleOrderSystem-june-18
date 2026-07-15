#include "OrderView.h"

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

std::string OrderView::ToDisplayString(OrderStatus status)
{
    switch (status) {
        case OrderStatus::RESERVED:
            return "RESERVED";
        case OrderStatus::REJECTED:
            return "REJECTED";
        case OrderStatus::PRODUCING:
            return "PRODUCING";
        case OrderStatus::CONFIRMED:
            return "CONFIRMED";
        case OrderStatus::RELEASE:
            return "RELEASE";
    }
    return "UNKNOWN";
}

void OrderView::ShowMenu() const
{
    std::cout << "\n[주문 메뉴]\n"
               << " 1. 주문 접수\n"
               << " 2. 접수된 주문 목록 조회\n"
               << " 3. 전체 주문 목록 조회\n"
               << " 4. 주문 승인\n"
               << " 5. 주문 거절\n"
               << " 0. 이전 메뉴로\n"
               << "선택 > ";
}

int OrderView::ReadMenuChoice() const
{
    return ReadIntOrInvalid("");
}

OrderReservationInputForm OrderView::ReadReservationInput() const
{
    OrderReservationInputForm input;
    input.sampleId = ReadIntOrInvalid("시료 ID: ");

    std::cout << "고객명: ";
    std::getline(std::cin, input.customerName);

    input.quantity = ReadIntOrInvalid("수량: ");

    return input;
}

int OrderView::ReadOrderId(const std::string& prompt) const
{
    return ReadIntOrInvalid(prompt);
}

void OrderView::ShowReservedOrders(const std::vector<Order>& orders) const
{
    if (orders.empty()) {
        std::cout << "접수된 주문이 없습니다\n";
        return;
    }
    for (const Order& order : orders) {
        std::cout << order.GetOrderId() << "\t" << order.GetSampleId() << "\t" << order.GetCustomerName() << "\t"
                   << order.GetQuantity() << "\n";
    }
}

void OrderView::ShowAllOrders(const std::vector<Order>& orders) const
{
    if (orders.empty()) {
        std::cout << "등록된 주문이 없습니다\n";
        return;
    }
    for (const Order& order : orders) {
        std::cout << order.GetOrderId() << "\t" << order.GetSampleId() << "\t" << order.GetCustomerName() << "\t"
                   << order.GetQuantity() << "\t" << OrderView::ToDisplayString(order.GetStatus()) << "\n";
    }
}

void OrderView::ShowMessage(const std::string& message) const
{
    std::cout << message << "\n";
}

}  // namespace order_system
