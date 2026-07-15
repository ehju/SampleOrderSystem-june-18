#include "OrderController.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

namespace order_system {

namespace {

bool IsBlank(const std::string& text)
{
    return std::all_of(text.begin(), text.end(), [](unsigned char ch) { return std::isspace(ch); });
}

}  // namespace

OrderController::OrderController(ISampleRepository& sampleRepository, IOrderRepository& orderRepository)
    : sampleRepository_(sampleRepository),
      orderRepository_(orderRepository)
{
}

OrderReservationResult OrderController::ReserveOrder(int sampleId, const std::string& customerName, int quantity)
{
    if (!sampleRepository_.FindById(sampleId).has_value()) {
        return { false, "존재하지 않는 시료입니다", std::nullopt };
    }
    if (IsBlank(customerName)) {
        return { false, "고객명은 공백일 수 없습니다", std::nullopt };
    }
    if (quantity < 1) {
        return { false, "주문 수량은 1 이상이어야 합니다", std::nullopt };
    }

    Order order = orderRepository_.Add(sampleId, customerName, quantity);
    return { true, "", order };
}

std::vector<Order> OrderController::GetReservedOrders() const
{
    std::vector<Order> reservedOrders = orderRepository_.FindByStatus(OrderStatus::RESERVED);
    std::sort(reservedOrders.begin(), reservedOrders.end(), [](const Order& lhs, const Order& rhs) {
        return lhs.GetCreatedAt() < rhs.GetCreatedAt();
    });
    return reservedOrders;
}

std::vector<Order> OrderController::GetAllOrders() const
{
    return orderRepository_.GetAll();
}

OrderApprovalResult OrderController::ApproveOrder(int orderId)
{
    std::optional<Order> order = orderRepository_.FindById(orderId);
    if (!order.has_value() || order->GetStatus() != OrderStatus::RESERVED) {
        return { false, "승인 가능한 주문이 아닙니다", std::nullopt };
    }

    std::optional<Sample> sample = sampleRepository_.FindById(order->GetSampleId());
    if (!sample.has_value()) {
        return { false, "존재하지 않는 시료입니다", std::nullopt };
    }

    if (sample->GetStockQuantity() >= order->GetQuantity()) {
        orderRepository_.UpdateStatus(orderId, OrderStatus::CONFIRMED);
        return { true, "", OrderStatus::CONFIRMED };
    }

    int shortageQuantity = order->GetQuantity() - sample->GetStockQuantity();
    int actualProductionQuantity =
        static_cast<int>(std::ceil(static_cast<double>(shortageQuantity) / sample->GetYield()));
    double totalProductionTime = sample->GetAvgProductionTime() * actualProductionQuantity;

    productionQueue_.push(ProductionJob(orderId, sample->GetSampleId(), shortageQuantity, actualProductionQuantity,
        totalProductionTime, std::chrono::system_clock::now(), 0));
    orderRepository_.UpdateStatus(orderId, OrderStatus::PRODUCING);
    return { true, "", OrderStatus::PRODUCING };
}

OrderApprovalResult OrderController::RejectOrder(int orderId)
{
    std::optional<Order> order = orderRepository_.FindById(orderId);
    if (!order.has_value() || order->GetStatus() != OrderStatus::RESERVED) {
        return { false, "거절 가능한 주문이 아닙니다", std::nullopt };
    }

    orderRepository_.UpdateStatus(orderId, OrderStatus::REJECTED);
    return { true, "", OrderStatus::REJECTED };
}

const std::queue<ProductionJob>& OrderController::GetProductionQueue() const
{
    return productionQueue_;
}

ProductionJob OrderController::PopProductionJob()
{
    if (productionQueue_.empty()) {
        throw std::out_of_range("생산 큐가 비어 있습니다");
    }

    ProductionJob job = productionQueue_.front();
    productionQueue_.pop();
    return job;
}

}  // namespace order_system
