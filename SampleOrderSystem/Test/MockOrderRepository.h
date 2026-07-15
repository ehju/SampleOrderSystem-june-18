#pragma once

#include <gmock/gmock.h>

#include "../Model/OrderRepository.h"

namespace order_system {

// OrderController 단위 테스트에서 OrderRepository 를 대체하기 위한 GoogleMock 목 객체.
class MockOrderRepository : public IOrderRepository {
public:
    MOCK_METHOD(Order, Add, (int sampleId, const std::string& customerName, int quantity), (override));
    MOCK_METHOD(std::vector<Order>, GetAll, (), (const, override));
    MOCK_METHOD(std::vector<Order>, FindByStatus, (OrderStatus status), (const, override));
    MOCK_METHOD(std::optional<Order>, FindById, (int orderId), (const, override));
    MOCK_METHOD(void, UpdateStatus, (int orderId, OrderStatus status), (override));
};

}  // namespace order_system
