#pragma once

#include <chrono>
#include <string>

#include "OrderStatus.h"

namespace order_system {

// Order entity.
// On construction, status is always initialized to OrderStatus::RESERVED.
class Order {
public:
    Order(int orderId, int sampleId, const std::string& customerName, int quantity,
          const std::chrono::system_clock::time_point& createdAt);

    int GetOrderId() const;
    int GetSampleId() const;
    const std::string& GetCustomerName() const;
    int GetQuantity() const;
    OrderStatus GetStatus() const;
    const std::chrono::system_clock::time_point& GetCreatedAt() const;

private:
    int orderId_;
    int sampleId_;
    std::string customerName_;
    int quantity_;
    OrderStatus status_;
    std::chrono::system_clock::time_point createdAt_;
};

}  // namespace order_system
