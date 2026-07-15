#include "OrderRepository.h"

#include <algorithm>
#include <stdexcept>

#include "../Lib/Json.h"

namespace order_system {

namespace {

std::string OrderStatusToString(OrderStatus status)
{
    switch (status) {
        case OrderStatus::RESERVED:
            return "RESERVED";
        case OrderStatus::REJECTED:
            return "REJECTED";
        case OrderStatus::PRODUCING:
            return "PRODUCING";
        case OrderStatus::CONFIRMED:
            return "CONFIRMED";
        case OrderStatus::RELEASE:
            return "RELEASE";
    }
    throw std::invalid_argument("알 수 없는 OrderStatus 입니다.");
}

OrderStatus OrderStatusFromString(const std::string& text)
{
    if (text == "RESERVED") {
        return OrderStatus::RESERVED;
    }
    if (text == "REJECTED") {
        return OrderStatus::REJECTED;
    }
    if (text == "PRODUCING") {
        return OrderStatus::PRODUCING;
    }
    if (text == "CONFIRMED") {
        return OrderStatus::CONFIRMED;
    }
    if (text == "RELEASE") {
        return OrderStatus::RELEASE;
    }
    throw std::invalid_argument("알 수 없는 OrderStatus 문자열입니다: " + text);
}

}  // namespace

OrderRepository::OrderRepository(std::filesystem::path jsonFilePath)
    : jsonFilePath_(std::move(jsonFilePath))
{
    LoadFromJsonFile();
}

void OrderRepository::LoadFromJsonFile()
{
    if (!std::filesystem::exists(jsonFilePath_)) {
        return;
    }

    json::JsonValue root = json::JsonValue::LoadFromFile(jsonFilePath_);
    for (const json::JsonValue& item : root.AsArray()) {
        int orderId = static_cast<int>(item.Find("orderId")->AsNumber());
        int sampleId = static_cast<int>(item.Find("sampleId")->AsNumber());
        std::string customerName = item.Find("customerName")->AsString();
        int quantity = static_cast<int>(item.Find("quantity")->AsNumber());
        OrderStatus status = OrderStatusFromString(item.Find("status")->AsString());
        long long createdAtEpochSeconds = static_cast<long long>(item.Find("createdAt")->AsNumber());

        Order order(orderId, sampleId, customerName, quantity,
            std::chrono::system_clock::time_point(std::chrono::seconds(createdAtEpochSeconds)));
        order.SetStatus(status);

        orders_.push_back(order);
        nextOrderId_ = std::max(nextOrderId_, orderId + 1);
    }
}

void OrderRepository::SaveToJsonFile() const
{
    json::JsonValue root = json::JsonValue::MakeArray();
    for (const Order& order : orders_) {
        long long createdAtEpochSeconds =
            std::chrono::duration_cast<std::chrono::seconds>(order.GetCreatedAt().time_since_epoch()).count();

        json::JsonValue item = json::JsonValue::MakeObject();
        item["orderId"] = order.GetOrderId();
        item["sampleId"] = order.GetSampleId();
        item["customerName"] = order.GetCustomerName();
        item["quantity"] = order.GetQuantity();
        item["status"] = OrderStatusToString(order.GetStatus());
        item["createdAt"] = static_cast<double>(createdAtEpochSeconds);
        root.Push(std::move(item));
    }

    std::filesystem::path parentDir = jsonFilePath_.parent_path();
    if (!parentDir.empty()) {
        std::filesystem::create_directories(parentDir);
    }
    root.SaveToFile(jsonFilePath_);
}

Order OrderRepository::Add(int sampleId, const std::string& customerName, int quantity)
{
    Order order(nextOrderId_, sampleId, customerName, quantity, std::chrono::system_clock::now());
    ++nextOrderId_;
    orders_.push_back(order);
    SaveToJsonFile();

    return order;
}

std::vector<Order> OrderRepository::GetAll() const
{
    return orders_;
}

std::vector<Order> OrderRepository::FindByStatus(OrderStatus status) const
{
    std::vector<Order> result;
    for (const Order& order : orders_) {
        if (order.GetStatus() == status) {
            result.push_back(order);
        }
    }
    return result;
}

std::optional<Order> OrderRepository::FindById(int orderId) const
{
    for (const Order& order : orders_) {
        if (order.GetOrderId() == orderId) {
            return order;
        }
    }
    return std::nullopt;
}

}  // namespace order_system
