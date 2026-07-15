#include "MainMenuController.h"

#include <string>
#include <vector>

#include "../Model/Sample.h"
#include "../View/MainMenuView.h"
#include "../View/MonitoringView.h"
#include "../View/OrderView.h"
#include "../View/ProductionView.h"
#include "../View/SampleView.h"
#include "../View/ShipmentView.h"

namespace order_system {

MainMenuController::MainMenuController(ISampleRepository& sampleRepository, SampleController& sampleController,
                                        OrderController& orderController, MonitoringController& monitoringController,
                                        ProductionController& productionController,
                                        ShipmentController& shipmentController, ProductionQueue& productionQueue)
    : sampleRepository_(sampleRepository),
      sampleController_(sampleController),
      orderController_(orderController),
      monitoringController_(monitoringController),
      productionController_(productionController),
      shipmentController_(shipmentController),
      productionQueue_(productionQueue)
{
}

void MainMenuController::Run()
{
    MainMenuView mainMenuView;
    bool running = true;

    while (running) {
        SyncProductionQueue();
        mainMenuView.ShowMainMenu(ComputeSummary());

        switch (mainMenuView.ReadMenuChoice()) {
            case 1:
                RunSampleMenu();
                break;
            case 2:
                RunOrderMenu();
                break;
            case 3:
                RunMonitoringMenu();
                break;
            case 4:
                RunShipmentMenu();
                break;
            case 5:
                RunProductionMenu();
                break;
            case 0:
                running = false;
                break;
            default:
                mainMenuView.ShowMessage("잘못된 입력입니다");
                break;
        }
    }
}

MainMenuSummary MainMenuController::ComputeSummary() const
{
    MainMenuSummary summary;

    std::vector<Sample> samples = sampleRepository_.GetAll();
    summary.registeredSampleCount = static_cast<int>(samples.size());
    for (const Sample& sample : samples) {
        summary.totalStockQuantity += sample.GetStockQuantity();
    }

    summary.orderStatusCounts = monitoringController_.GetOrderStatusCounts();
    return summary;
}

void MainMenuController::SyncProductionQueue()
{
    while (!orderController_.GetProductionQueue().empty()) {
        productionQueue_.Enqueue(orderController_.PopProductionJob());
    }
}

void MainMenuController::RunSampleMenu()
{
    SampleView sampleView;
    bool running = true;

    while (running) {
        sampleView.ShowMenu();
        switch (sampleView.ReadMenuChoice()) {
            case 1: {
                SampleRegistrationInput input = sampleView.ReadSampleRegistrationInput();
                SampleRegistrationResult result =
                    sampleController_.RegisterSample(input.name, input.avgProductionTime, input.yield);
                sampleView.ShowMessage(result.success ? "시료가 등록되었습니다" : result.errorMessage);
                break;
            }
            case 2:
                sampleView.ShowSampleList(sampleRepository_.GetAll());
                break;
            case 3: {
                std::string keyword = sampleView.ReadSearchKeyword();
                sampleView.ShowSearchResults(sampleRepository_.SearchByName(keyword));
                break;
            }
            case 0:
                running = false;
                break;
            default:
                sampleView.ShowMessage("잘못된 입력입니다");
                break;
        }
    }
}

void MainMenuController::RunOrderMenu()
{
    OrderView orderView;
    bool running = true;

    while (running) {
        orderView.ShowMenu();
        switch (orderView.ReadMenuChoice()) {
            case 1: {
                OrderReservationInputForm input = orderView.ReadReservationInput();
                OrderReservationResult result =
                    orderController_.ReserveOrder(input.sampleId, input.customerName, input.quantity);
                orderView.ShowMessage(result.success ? "주문이 접수되었습니다" : result.errorMessage);
                break;
            }
            case 2:
                orderView.ShowReservedOrders(orderController_.GetReservedOrders());
                break;
            case 3:
                orderView.ShowAllOrders(orderController_.GetAllOrders());
                break;
            case 4: {
                int orderId = orderView.ReadOrderId("승인할 주문 ID: ");
                OrderApprovalResult result = orderController_.ApproveOrder(orderId);
                orderView.ShowMessage(result.success
                                          ? "승인 처리되었습니다. 상태: " + OrderView::ToDisplayString(*result.newStatus)
                                          : result.errorMessage);
                break;
            }
            case 5: {
                int orderId = orderView.ReadOrderId("거절할 주문 ID: ");
                OrderApprovalResult result = orderController_.RejectOrder(orderId);
                orderView.ShowMessage(result.success ? "거절 처리되었습니다" : result.errorMessage);
                break;
            }
            case 0:
                running = false;
                break;
            default:
                orderView.ShowMessage("잘못된 입력입니다");
                break;
        }
    }
}

void MainMenuController::RunMonitoringMenu()
{
    MonitoringView monitoringView;
    bool running = true;

    while (running) {
        monitoringView.ShowMenu();
        switch (monitoringView.ReadMenuChoice()) {
            case 1:
                monitoringView.ShowOrderStatus(monitoringController_.GetOrderStatusCounts());
                break;
            case 2:
                monitoringView.ShowStockStatus(monitoringController_.GetStockStatuses());
                break;
            case 0:
                running = false;
                break;
            default:
                monitoringView.ShowMessage("잘못된 입력입니다");
                break;
        }
    }
}

void MainMenuController::RunShipmentMenu()
{
    ShipmentView shipmentView;
    bool running = true;

    while (running) {
        shipmentView.ShowMenu();
        switch (shipmentView.ReadMenuChoice()) {
            case 1:
                shipmentView.ShowShipmentTargets(shipmentController_.GetShipmentTargets());
                break;
            case 2: {
                int orderId = shipmentView.ReadOrderId("출고할 주문 ID: ");
                ShipmentResult result = shipmentController_.ProcessShipment(orderId);
                shipmentView.ShowMessage(result.success ? "출고 처리되었습니다" : result.errorMessage);
                break;
            }
            case 0:
                running = false;
                break;
            default:
                shipmentView.ShowMessage("잘못된 입력입니다");
                break;
        }
    }
}

void MainMenuController::RunProductionMenu()
{
    ProductionView productionView;
    bool running = true;

    while (running) {
        productionView.ShowMenu();
        switch (productionView.ReadMenuChoice()) {
            case 1:
                productionView.ShowQueue(productionQueue_.GetAll());
                break;
            case 2: {
                ProductionCompletionResult result = productionController_.CompleteProduction();
                if (result.success) {
                    productionView.ShowCompletedJob(*result.completedJob);
                } else {
                    productionView.ShowMessage(result.errorMessage);
                }
                break;
            }
            case 0:
                running = false;
                break;
            default:
                productionView.ShowMessage("잘못된 입력입니다");
                break;
        }
    }
}

}  // namespace order_system
