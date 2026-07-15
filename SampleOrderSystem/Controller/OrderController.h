#pragma once

#include <optional>
#include <string>

#include "../Model/Order.h"
#include "../Model/OrderRepository.h"
#include "../Model/SampleRepository.h"

namespace order_system {

// 주문 접수(예약) 요청 처리 결과.
struct OrderReservationResult {
    bool success;
    std::string errorMessage;
    std::optional<Order> order;
};

// OrderController: 예약(접수) 유스케이스를 담당한다.
// sampleId 유효성은 SampleRepository 로 확인하고, 검증 통과 시 OrderRepository 에 생성을 위임한다.
class OrderController {
public:
    OrderController(ISampleRepository& sampleRepository, IOrderRepository& orderRepository);

    // 시료 ID/고객명/주문 수량을 검증한 뒤 유효하면 orderRepository 에 생성을 위임한다.
    // 검증에 실패하면 등록을 거부하고 실패 사유를 결과에 담는다.
    OrderReservationResult ReserveOrder(int sampleId, const std::string& customerName, int quantity);

private:
    ISampleRepository& sampleRepository_;
    IOrderRepository& orderRepository_;
};

}  // namespace order_system
