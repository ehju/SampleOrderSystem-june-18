#pragma once

#include <cstddef>
#include <vector>

#include "ProductionJob.h"

namespace order_system {

// 생산 큐: 승인 처리 중 재고 부족으로 생성된 ProductionJob 을 FIFO 순서로 관리한다.
// JSON 파일로 영속화하지 않는 인메모리 런타임 상태다 (프로그램 종료 시 사라져도 무방하다).
class ProductionQueue {
public:
    ProductionQueue() = default;

    // job 을 큐의 맨 뒤에 추가한다.
    void Enqueue(const ProductionJob& job);

    // 큐 선두의 job 을 꺼내 반환하고 큐에서 제거한다. 큐가 비어 있으면 std::out_of_range 를 던진다.
    ProductionJob Dequeue();

    // 큐 선두의 job 을 제거하지 않고 참조로 반환한다. 큐가 비어 있으면 std::out_of_range 를 던진다.
    const ProductionJob& Peek() const;

    // 큐가 비어 있는지 확인한다.
    bool IsEmpty() const;

    // 큐에 쌓인 job 개수를 반환한다.
    size_t Size() const;

    // 큐에 쌓인 job 전체를 FIFO 순서(삽입 순서)대로 반환한다 (대기열 조회용, 큐 상태는 변경하지 않는다).
    std::vector<ProductionJob> GetAll() const;

private:
    std::vector<ProductionJob> jobs_;
};

}  // namespace order_system
