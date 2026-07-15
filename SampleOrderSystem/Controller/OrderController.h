#pragma once

#include <optional>
#include <queue>
#include <string>
#include <vector>

#include "../Model/Order.h"
#include "../Model/OrderRepository.h"
#include "../Model/ProductionJob.h"
#include "../Model/SampleRepository.h"

namespace order_system {

// 주문 접수(예약) 요청 처리 결과.
struct OrderReservationResult {
    bool success;
    std::string errorMessage;
    std::optional<Order> order;
};

// 주문 승인/거절 처리 결과.
struct OrderApprovalResult {
    bool success;
    std::string errorMessage;
    std::optional<OrderStatus> newStatus;
};

// OrderController: 예약(접수)/승인/거절 유스케이스를 담당한다.
// sampleId 유효성은 SampleRepository 로 확인하고, 검증 통과 시 OrderRepository 에 생성/상태 변경을 위임한다.
class OrderController {
public:
    OrderController(ISampleRepository& sampleRepository, IOrderRepository& orderRepository);

    // 시료 ID/고객명/주문 수량을 검증한 뒤 유효하면 orderRepository 에 생성을 위임한다.
    // 검증에 실패하면 등록을 거부하고 실패 사유를 결과에 담는다.
    OrderReservationResult ReserveOrder(int sampleId, const std::string& customerName, int quantity);

    // RESERVED 상태의 주문을 접수 순서(createdAt 오름차순, FIFO) 로 정렬하여 반환한다.
    std::vector<Order> GetReservedOrders() const;

    // 대상 주문이 RESERVED 상태인지 확인한 뒤, 재고 판정 결과에 따라 CONFIRMED 또는 PRODUCING 으로 전환한다.
    // 재고가 부족하면 ProductionJob 을 생성해 생산 큐(FIFO) 맨 뒤에 추가한다.
    // 정책 B(출고 시 차감)에 따라 이 처리에서는 재고를 차감하지 않는다.
    OrderApprovalResult ApproveOrder(int orderId);

    // 대상 주문이 RESERVED 상태인지 확인한 뒤 REJECTED 로 전환한다. 재고/생산 큐에는 영향을 주지 않는다.
    OrderApprovalResult RejectOrder(int orderId);

    // 승인 처리 중 생성된 ProductionJob 이 쌓이는 생산 큐(FIFO). 실제 소비 로직은 Phase 5 가 담당한다.
    const std::queue<ProductionJob>& GetProductionQueue() const;

private:
    ISampleRepository& sampleRepository_;
    IOrderRepository& orderRepository_;
    std::queue<ProductionJob> productionQueue_;
};

}  // namespace order_system
