#pragma once

#include <string>
#include <vector>

#include "../Model/ProductionJob.h"

namespace order_system {

// ProductionView: 생산 라인(대기열 조회/생산 완료 처리)과 관련된 화면 출력과 입력 수신만 담당한다.
class ProductionView {
public:
    void ShowMenu() const;
    int ReadMenuChoice() const;

    // 생산 큐 전체를 FIFO 순서(삽입 순서)대로 출력한다. 표시 항목: orderId, sampleId, shortageQuantity,
    // actualProductionQuantity, totalProductionTime, 큐 내 순번. jobs 가 비어있으면 "대기 중인 생산 작업이 없습니다" 를 출력한다.
    void ShowQueue(const std::vector<ProductionJob>& jobs) const;

    // 생산 완료 처리 결과를 orderId, sampleId, actualProductionQuantity 로 출력한다.
    void ShowCompletedJob(const ProductionJob& job) const;

    void ShowMessage(const std::string& message) const;
};

}  // namespace order_system
