#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "Order.h"
#include "OrderStatus.h"

namespace order_system {

// OrderRepository 의 추상 인터페이스.
// OrderController 단위 테스트에서 GoogleMock 으로 대체하기 위해 사용한다.
class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;

    // 새 Order 를 생성한다. orderId 는 자동 채번하고, status 는 RESERVED,
    // createdAt 은 현재 처리 시각으로 설정한다.
    virtual Order Add(int sampleId, const std::string& customerName, int quantity) = 0;

    // 생성된 모든 Order 목록을 반환한다.
    virtual std::vector<Order> GetAll() const = 0;

    // status 에 해당하는 Order 목록만 반환한다.
    virtual std::vector<Order> FindByStatus(OrderStatus status) const = 0;

    // orderId 로 단건 조회한다. 존재하지 않으면 std::nullopt 를 반환한다.
    virtual std::optional<Order> FindById(int orderId) const = 0;
};

// JSON 파일 기반으로 Order 를 영속화하는 Repository 구현체.
// 생성 시 jsonFilePath 가 존재하면 그 내용을 읽어 초기 상태를 구성하고,
// 생성(및 이후 phase 의 상태 변경) 등 변경이 있을 때마다 즉시 JSON 파일에 반영한다.
class OrderRepository : public IOrderRepository {
public:
    explicit OrderRepository(std::filesystem::path jsonFilePath);

    Order Add(int sampleId, const std::string& customerName, int quantity) override;
    std::vector<Order> GetAll() const override;
    std::vector<Order> FindByStatus(OrderStatus status) const override;
    std::optional<Order> FindById(int orderId) const override;

private:
    // jsonFilePath_ 가 존재하면 그 내용을 읽어 orders_/nextOrderId_ 를 구성한다.
    void LoadFromJsonFile();

    // orders_ 의 현재 상태를 jsonFilePath_ 에 즉시 반영한다.
    void SaveToJsonFile() const;

    std::filesystem::path jsonFilePath_;
    std::vector<Order> orders_;
    int nextOrderId_ = 1;
};

}  // namespace order_system
