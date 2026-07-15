#pragma once

#include "../Model/ProductionQueue.h"
#include "../Model/SampleRepository.h"
#include "MonitoringController.h"
#include "OrderController.h"
#include "ProductionController.h"
#include "SampleController.h"
#include "ShipmentController.h"

namespace order_system {

// 메인 메뉴 요약 정보.
struct MainMenuSummary {
    int registeredSampleCount = 0;
    int totalStockQuantity = 0;
    OrderStatusCounts orderStatusCounts;
};

// MainMenuController: 메인 메뉴 요약 정보 계산과 하위 메뉴(1~5) 라우팅을 담당한다.
// 병렬로 진행된 Phase 5/6/7 트랙이 합류하는 지점으로, 각 하위 Controller 를 호출만 하고
// 도메인 로직은 직접 수행하지 않는다.
class MainMenuController {
public:
    MainMenuController(ISampleRepository& sampleRepository, SampleController& sampleController,
                        OrderController& orderController, MonitoringController& monitoringController,
                        ProductionController& productionController, ShipmentController& shipmentController,
                        ProductionQueue& productionQueue);

    // 메인 메뉴 루프를 실행한다. 0 입력 시 반환한다.
    void Run();

private:
    // 등록 시료 수/총 재고 수량/상태별(RESERVED/PRODUCING/CONFIRMED) 주문 수 요약을 계산한다.
    MainMenuSummary ComputeSummary() const;

    // OrderController 내부 생산 큐에 쌓인 ProductionJob 을 공유 ProductionQueue(Model)로 이관한다.
    // OrderController(Phase 4)와 ProductionController(Phase 5)가 각자 다른 큐 타입을 사용하므로,
    // 두 병렬 트랙이 합류하는 이 지점에서 연결해준다.
    void SyncProductionQueue();

    void RunSampleMenu();
    void RunOrderMenu();
    void RunMonitoringMenu();
    void RunShipmentMenu();
    void RunProductionMenu();

    ISampleRepository& sampleRepository_;
    SampleController& sampleController_;
    OrderController& orderController_;
    MonitoringController& monitoringController_;
    ProductionController& productionController_;
    ShipmentController& shipmentController_;
    ProductionQueue& productionQueue_;
};

}  // namespace order_system
