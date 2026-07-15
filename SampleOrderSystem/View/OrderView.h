#pragma once

#include <string>
#include <vector>

#include "../Model/Order.h"

namespace order_system {

// 주문 접수 입력폼에서 받은 값.
struct OrderReservationInputForm {
    int sampleId;
    std::string customerName;
    int quantity;
};

// OrderView: 주문 접수/승인/거절과 관련된 화면 출력과 입력 수신만 담당한다.
class OrderView {
public:
    void ShowMenu() const;
    int ReadMenuChoice() const;

    // OrderStatus 를 사람이 읽을 수 있는 문자열로 변환한다.
    static std::string ToDisplayString(OrderStatus status);

    // 시료 ID/고객명/수량을 순서대로 입력받는다. 숫자 입력에 실패하면 -1로 채운다.
    OrderReservationInputForm ReadReservationInput() const;

    // 승인/거절 대상 주문 ID를 입력받는다. 숫자 입력에 실패하면 -1을 반환한다.
    int ReadOrderId(const std::string& prompt) const;

    // 접수(RESERVED) 주문 목록을 orderId, sampleId, customerName, quantity 컬럼으로 출력한다.
    // orders 가 비어있으면 "접수된 주문이 없습니다" 를 출력한다.
    void ShowReservedOrders(const std::vector<Order>& orders) const;

    // 전체 주문 목록을 orderId, sampleId, customerName, quantity, status 컬럼으로 출력한다.
    // orders 가 비어있으면 "등록된 주문이 없습니다" 를 출력한다.
    void ShowAllOrders(const std::vector<Order>& orders) const;

    void ShowMessage(const std::string& message) const;
};

}  // namespace order_system
