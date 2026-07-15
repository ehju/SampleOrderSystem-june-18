#include <gtest/gtest.h>

#include <filesystem>
#include <optional>
#include <string>

#include "../Controller/MonitoringController.h"
#include "../Controller/OrderController.h"
#include "../Controller/ProductionController.h"
#include "../Controller/SampleController.h"
#include "../Controller/ShipmentController.h"
#include "../Model/OrderRepository.h"
#include "../Model/ProductionQueue.h"
#include "../Model/SampleRepository.h"

using order_system::MonitoringController;
using order_system::Order;
using order_system::OrderController;
using order_system::OrderRepository;
using order_system::OrderStatus;
using order_system::ProductionController;
using order_system::ProductionQueue;
using order_system::Sample;
using order_system::SampleController;
using order_system::SampleRepository;
using order_system::ShipmentController;

namespace {

namespace fs = std::filesystem;

// Phase 9 핵심 통합 시나리오: Repository 는 실제 구현을 사용하고 각 Controller 를 엮어 end-to-end 로 검증한다.
class IntegrationScenarioTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        testDir_ = fs::temp_directory_path() / "SampleOrderSystemTest" /
            ::testing::UnitTest::GetInstance()->current_test_info()->name();

        std::error_code ec;
        fs::remove_all(testDir_, ec);
        fs::create_directories(testDir_, ec);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(testDir_, ec);
    }

    // OrderController 내부 생산 큐에 쌓인 ProductionJob 을 공유 ProductionQueue 로 이관한다.
    void SyncProductionQueue(OrderController& orderController, ProductionQueue& productionQueue)
    {
        while (!orderController.GetProductionQueue().empty()) {
            productionQueue.Enqueue(orderController.PopProductionJob());
        }
    }

    fs::path testDir_;
};

}  // namespace

// 핵심 시나리오 1: 시료 등록 -> 주문 접수 -> 승인(재고 충분) -> 출고 -> 모니터링 확인.
TEST_F(IntegrationScenarioTest, ReservationApprovalWithSufficientStockThenShipmentIsReflectedInMonitoring)
{
    SampleRepository sampleRepository(testDir_ / "samples.json");
    OrderRepository orderRepository(testDir_ / "orders.json");
    SampleController sampleController(sampleRepository);
    OrderController orderController(sampleRepository, orderRepository);
    ShipmentController shipmentController(sampleRepository, orderRepository);
    MonitoringController monitoringController(sampleRepository, orderRepository);

    ASSERT_TRUE(sampleController.RegisterSample("SampleA", 10, 1.0).success);
    Sample sample = sampleRepository.GetAll().front();
    sampleRepository.IncreaseStock(sample.GetSampleId(), 50);

    auto reservation = orderController.ReserveOrder(sample.GetSampleId(), "CustomerA", 20);
    ASSERT_TRUE(reservation.success);
    int orderId = reservation.order->GetOrderId();

    auto approval = orderController.ApproveOrder(orderId);
    ASSERT_TRUE(approval.success);
    EXPECT_EQ(*approval.newStatus, OrderStatus::CONFIRMED);

    auto shipment = shipmentController.ProcessShipment(orderId);
    ASSERT_TRUE(shipment.success);

    auto counts = monitoringController.GetOrderStatusCounts();
    EXPECT_EQ(counts.releaseCount, 1);
    EXPECT_EQ(counts.reservedCount, 0);
    EXPECT_EQ(counts.producingCount, 0);
    EXPECT_EQ(counts.confirmedCount, 0);

    auto stockStatuses = monitoringController.GetStockStatuses();
    ASSERT_EQ(stockStatuses.size(), 1u);
    EXPECT_EQ(stockStatuses.front().stockQuantity, 30);
    EXPECT_EQ(stockStatuses.front().pendingQuantity, 0);
}

// 핵심 시나리오 2: 시료 등록(낮은 재고) -> 주문 접수(재고 초과) -> 승인(PRODUCING) -> 생산 완료(CONFIRMED) -> 출고 -> 모니터링 확인.
TEST_F(IntegrationScenarioTest, ReservationApprovalWithInsufficientStockThenProductionAndShipmentIsReflected)
{
    SampleRepository sampleRepository(testDir_ / "samples.json");
    OrderRepository orderRepository(testDir_ / "orders.json");
    ProductionQueue productionQueue;
    SampleController sampleController(sampleRepository);
    OrderController orderController(sampleRepository, orderRepository);
    ProductionController productionController(productionQueue, sampleRepository, orderRepository);
    ShipmentController shipmentController(sampleRepository, orderRepository);
    MonitoringController monitoringController(sampleRepository, orderRepository);

    ASSERT_TRUE(sampleController.RegisterSample("SampleB", 5, 0.5).success);
    Sample sample = sampleRepository.GetAll().front();

    auto reservation = orderController.ReserveOrder(sample.GetSampleId(), "CustomerB", 10);
    ASSERT_TRUE(reservation.success);
    int orderId = reservation.order->GetOrderId();

    auto approval = orderController.ApproveOrder(orderId);
    ASSERT_TRUE(approval.success);
    EXPECT_EQ(*approval.newStatus, OrderStatus::PRODUCING);

    SyncProductionQueue(orderController, productionQueue);
    ASSERT_FALSE(productionQueue.IsEmpty());

    auto completion = productionController.CompleteProduction();
    ASSERT_TRUE(completion.success);

    auto orderAfterCompletion = orderRepository.FindById(orderId);
    ASSERT_TRUE(orderAfterCompletion.has_value());
    EXPECT_EQ(orderAfterCompletion->GetStatus(), OrderStatus::CONFIRMED);

    auto shipment = shipmentController.ProcessShipment(orderId);
    ASSERT_TRUE(shipment.success);

    auto counts = monitoringController.GetOrderStatusCounts();
    EXPECT_EQ(counts.releaseCount, 1);
    EXPECT_EQ(counts.producingCount, 0);
    EXPECT_EQ(counts.confirmedCount, 0);

    auto stockStatuses = monitoringController.GetStockStatuses();
    ASSERT_EQ(stockStatuses.size(), 1u);
    // shortage=10, yield=0.5 -> actualProductionQuantity = ceil(10/0.5) = 20 -> 20 - 10(shipped) = 10 remains.
    EXPECT_EQ(stockStatuses.front().stockQuantity, 10);
    EXPECT_EQ(stockStatuses.front().pendingQuantity, 0);
}

// 핵심 시나리오 3: 주문 접수 -> 거절 -> 모니터링에서 REJECTED 가 어떤 집계에도 포함되지 않는다.
TEST_F(IntegrationScenarioTest, RejectedOrderIsExcludedFromAllMonitoringAggregations)
{
    SampleRepository sampleRepository(testDir_ / "samples.json");
    OrderRepository orderRepository(testDir_ / "orders.json");
    SampleController sampleController(sampleRepository);
    OrderController orderController(sampleRepository, orderRepository);
    MonitoringController monitoringController(sampleRepository, orderRepository);

    ASSERT_TRUE(sampleController.RegisterSample("SampleC", 10, 1.0).success);
    Sample sample = sampleRepository.GetAll().front();

    auto reservation = orderController.ReserveOrder(sample.GetSampleId(), "CustomerC", 5);
    ASSERT_TRUE(reservation.success);
    int orderId = reservation.order->GetOrderId();

    auto rejection = orderController.RejectOrder(orderId);
    ASSERT_TRUE(rejection.success);
    EXPECT_EQ(*rejection.newStatus, OrderStatus::REJECTED);

    auto counts = monitoringController.GetOrderStatusCounts();
    EXPECT_EQ(counts.reservedCount, 0);
    EXPECT_EQ(counts.producingCount, 0);
    EXPECT_EQ(counts.confirmedCount, 0);
    EXPECT_EQ(counts.releaseCount, 0);

    auto stockStatuses = monitoringController.GetStockStatuses();
    ASSERT_EQ(stockStatuses.size(), 1u);
    EXPECT_EQ(stockStatuses.front().pendingQuantity, 0);
}
