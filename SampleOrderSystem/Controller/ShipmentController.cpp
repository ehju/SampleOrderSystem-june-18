#include "ShipmentController.h"

namespace order_system {

ShipmentController::ShipmentController(ISampleRepository& sampleRepository, IOrderRepository& orderRepository)
    : sampleRepository_(sampleRepository),
      orderRepository_(orderRepository)
{
}

std::vector<Order> ShipmentController::GetShipmentTargets() const
{
    return orderRepository_.FindByStatus(OrderStatus::CONFIRMED);
}

ShipmentResult ShipmentController::ProcessShipment(int orderId)
{
    std::optional<Order> order = orderRepository_.FindById(orderId);
    if (!order.has_value() || order->GetStatus() != OrderStatus::CONFIRMED) {
        return { false, "출고 가능한 주문이 아닙니다" };
    }

    std::optional<Sample> sample = sampleRepository_.FindById(order->GetSampleId());
    if (!sample.has_value() || sample->GetStockQuantity() < order->GetQuantity()) {
        return { false, "재고가 부족하여 출고할 수 없습니다" };
    }

    sampleRepository_.DecreaseStock(sample->GetSampleId(), order->GetQuantity());
    orderRepository_.UpdateStatus(order->GetOrderId(), OrderStatus::RELEASE);

    return { true, "" };
}

}  // namespace order_system
