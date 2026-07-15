#pragma once

#include <string>
#include <vector>

#include "../Model/Order.h"

namespace order_system {

// ShipmentView: 출고 처리(대상 조회/출고 실행)와 관련된 화면 출력과 입력 수신만 담당한다.
class ShipmentView {
public:
    void ShowMenu() const;
    int ReadMenuChoice() const;

    // 출고할 주문 ID를 입력받는다. 숫자 입력에 실패하면 -1을 반환한다.
    int ReadOrderId(const std::string& prompt) const;

    // 출고 대상(CONFIRMED) 주문 목록을 orderId, sampleId, customerName, quantity 컬럼으로 출력한다.
    // orders 가 비어있으면 "출고 대상 주문이 없습니다" 를 출력한다.
    void ShowShipmentTargets(const std::vector<Order>& orders) const;

    void ShowMessage(const std::string& message) const;
};

}  // namespace order_system
