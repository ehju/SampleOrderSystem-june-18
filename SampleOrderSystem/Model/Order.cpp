#include "Order.h"

namespace order_system {

Order::Order(int orderId, int sampleId, const std::string& customerName, int quantity,
             const std::chrono::system_clock::time_point& createdAt)
    : orderId_(orderId),
      sampleId_(sampleId),
      customerName_(customerName),
      quantity_(quantity),
      status_(OrderStatus::RESERVED),
      createdAt_(createdAt)
{
}

int Order::GetOrderId() const
{
    return orderId_;
}

int Order::GetSampleId() const
{
    return sampleId_;
}

const std::string& Order::GetCustomerName() const
{
    return customerName_;
}

int Order::GetQuantity() const
{
    return quantity_;
}

OrderStatus Order::GetStatus() const
{
    return status_;
}

const std::chrono::system_clock::time_point& Order::GetCreatedAt() const
{
    return createdAt_;
}

void Order::SetStatus(OrderStatus status)
{
    status_ = status;
}

}  // namespace order_system
