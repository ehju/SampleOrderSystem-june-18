#pragma once

#include <optional>
#include <string>

#include "../Model/OrderRepository.h"
#include "../Model/ProductionJob.h"
#include "../Model/ProductionQueue.h"
#include "../Model/SampleRepository.h"

namespace order_system {

// 생산 완료 처리 결과.
struct ProductionCompletionResult {
    bool success;
    std::string errorMessage;
    std::optional<ProductionJob> completedJob;
};

// ProductionController: 생산 큐(FIFO) 소비 및 생산 완료(수동 트리거) 유스케이스를 담당한다.
class ProductionController {
public:
    ProductionController(ProductionQueue& productionQueue, ISampleRepository& sampleRepository,
                          IOrderRepository& orderRepository);

    // 생산 큐 선두의 ProductionJob 을 꺼내 완료 처리한다.
    // 1) 대상 Sample 의 stockQuantity 에 actualProductionQuantity 를 더한다.
    // 2) 대상 Order 의 상태를 PRODUCING -> CONFIRMED 로 전환한다.
    // 3) Job 을 큐에서 제거한다.
    // 큐가 비어 있으면 처리를 거부하고 안내 메시지를 담아 반환한다.
    ProductionCompletionResult CompleteProduction();

private:
    ProductionQueue& productionQueue_;
    ISampleRepository& sampleRepository_;
    IOrderRepository& orderRepository_;
};

}  // namespace order_system
