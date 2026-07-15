#pragma once

#include <string>
#include <vector>

#include "../Model/Order.h"
#include "../Model/OrderRepository.h"
#include "../Model/SampleRepository.h"

namespace order_system {

// 출고 처리 결과.
struct ShipmentResult {
    bool success;
    std::string errorMessage;
};

// ShipmentController: CONFIRMED 상태 주문의 출고 실행 유스케이스를 담당한다.
// 재고 차감 시점 정책 B(출고 시 차감)에 따라, 이 컨트롤러에서 재고 재확인 및 차감을 수행한다.
class ShipmentController {
public:
    ShipmentController(ISampleRepository& sampleRepository, IOrderRepository& orderRepository);

    // CONFIRMED 상태의 주문 목록을 출고 대상으로 반환한다.
    std::vector<Order> GetShipmentTargets() const;

    // 대상 주문이 CONFIRMED 상태인지 확인한 뒤, 재고를 재확인하여 충분하면 차감하고
    // 주문 상태를 RELEASE 로 전환한다. 재고가 부족하면 차감 없이 처리를 거부하고
    // 주문 상태는 CONFIRMED 로 유지한다.
    ShipmentResult ProcessShipment(int orderId);

private:
    ISampleRepository& sampleRepository_;
    IOrderRepository& orderRepository_;
};

}  // namespace order_system
