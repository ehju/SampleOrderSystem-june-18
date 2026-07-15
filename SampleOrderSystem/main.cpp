#include <filesystem>
#include <iostream>
#include <limits>
#include <string>

#include "Controller/OrderController.h"
#include "Controller/SampleController.h"
#include "Model/OrderRepository.h"
#include "Model/SampleRepository.h"
#include "View/SampleView.h"

// Phase 0~4 임시 콘솔 확인용 진입점. 정식 메인 메뉴 통합은 Phase 8에서 별도로 진행한다.
using namespace order_system;

namespace {

std::string OrderStatusToDisplayString(OrderStatus status)
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
    return "UNKNOWN";
}

int ReadInt(const std::string& prompt)
{
    std::cout << prompt;
    int value = 0;
    std::cin >> value;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

std::string ReadLine(const std::string& prompt)
{
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

void ShowReservedOrders(const OrderController& orderController)
{
    std::vector<Order> reservedOrders = orderController.GetReservedOrders();
    if (reservedOrders.empty()) {
        std::cout << "접수된 주문이 없습니다\n";
        return;
    }
    for (const Order& order : reservedOrders) {
        std::cout << order.GetOrderId() << "\t" << order.GetSampleId() << "\t" << order.GetCustomerName() << "\t"
                   << order.GetQuantity() << "\n";
    }
}

void ShowAllOrders(const IOrderRepository& orderRepository)
{
    std::vector<Order> orders = orderRepository.GetAll();
    if (orders.empty()) {
        std::cout << "등록된 주문이 없습니다\n";
        return;
    }
    for (const Order& order : orders) {
        std::cout << order.GetOrderId() << "\t" << order.GetSampleId() << "\t" << order.GetCustomerName() << "\t"
                   << order.GetQuantity() << "\t" << OrderStatusToDisplayString(order.GetStatus()) << "\n";
    }
}

void ShowMenu()
{
    std::cout << "\n=== SampleOrderSystem 임시 확인 메뉴 (Phase 0~4) ===\n"
               << "1. 시료 등록\n"
               << "2. 시료 전체 조회\n"
               << "3. 시료 검색\n"
               << "4. 주문 접수\n"
               << "5. 접수된 주문 목록 조회\n"
               << "6. 전체 주문 목록 조회\n"
               << "7. 주문 승인\n"
               << "8. 주문 거절\n"
               << "0. 종료\n"
               << "선택: ";
}

}  // namespace

int main()
{
    std::filesystem::create_directories("data");
    SampleRepository sampleRepository("data/samples.json");
    OrderRepository orderRepository("data/orders.json");
    SampleController sampleController(sampleRepository);
    OrderController orderController(sampleRepository, orderRepository);
    SampleView sampleView;

    bool running = true;
    while (running) {
        ShowMenu();
        int choice = 0;
        std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        switch (choice) {
            case 1: {
                SampleRegistrationInput input = sampleView.ReadSampleRegistrationInput();
                SampleRegistrationResult result =
                    sampleController.RegisterSample(input.name, input.avgProductionTime, input.yield);
                sampleView.ShowMessage(result.success ? "시료가 등록되었습니다" : result.errorMessage);
                break;
            }
            case 2:
                sampleView.ShowSampleList(sampleRepository.GetAll());
                break;
            case 3: {
                std::string keyword = sampleView.ReadSearchKeyword();
                sampleView.ShowSearchResults(sampleRepository.SearchByName(keyword));
                break;
            }
            case 4: {
                int sampleId = ReadInt("시료 ID: ");
                std::string customerName = ReadLine("고객명: ");
                int quantity = ReadInt("수량: ");
                OrderReservationResult result = orderController.ReserveOrder(sampleId, customerName, quantity);
                std::cout << (result.success ? "주문이 접수되었습니다" : result.errorMessage) << "\n";
                break;
            }
            case 5:
                ShowReservedOrders(orderController);
                break;
            case 6:
                ShowAllOrders(orderRepository);
                break;
            case 7: {
                int orderId = ReadInt("승인할 주문 ID: ");
                OrderApprovalResult result = orderController.ApproveOrder(orderId);
                std::cout << (result.success
                                   ? "승인 처리되었습니다. 상태: " + OrderStatusToDisplayString(*result.newStatus)
                                   : result.errorMessage)
                           << "\n";
                break;
            }
            case 8: {
                int orderId = ReadInt("거절할 주문 ID: ");
                OrderApprovalResult result = orderController.RejectOrder(orderId);
                std::cout << (result.success ? "거절 처리되었습니다" : result.errorMessage) << "\n";
                break;
            }
            case 0:
                running = false;
                break;
            default:
                std::cout << "잘못된 입력입니다. 다시 선택해주세요.\n";
                break;
        }
    }

    return 0;
}
