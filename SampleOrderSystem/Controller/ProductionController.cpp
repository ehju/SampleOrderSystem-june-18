#include "ProductionController.h"

namespace order_system {

ProductionController::ProductionController(ProductionQueue& productionQueue, ISampleRepository& sampleRepository,
                                             IOrderRepository& orderRepository)
    : productionQueue_(productionQueue), sampleRepository_(sampleRepository), orderRepository_(orderRepository)
{
}

ProductionCompletionResult ProductionController::CompleteProduction()
{
    if (productionQueue_.IsEmpty()) {
        return ProductionCompletionResult{false, "진행 중인 생산이 없습니다", std::nullopt};
    }

    ProductionJob completedJob = productionQueue_.Dequeue();

    sampleRepository_.IncreaseStock(completedJob.GetSampleId(), completedJob.GetActualProductionQuantity());
    orderRepository_.UpdateStatus(completedJob.GetOrderId(), OrderStatus::CONFIRMED);

    return ProductionCompletionResult{true, "", completedJob};
}

}  // namespace order_system
