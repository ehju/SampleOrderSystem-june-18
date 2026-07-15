#include "OrderController.h"

#include <algorithm>
#include <cctype>

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

}  // namespace order_system
