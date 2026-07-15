# Phase 5 — 생산 라인 기능 (FIFO 큐 & 생산 완료 처리)

> 참조 명세 원문: [docs/specs/05-production-line.md](../docs/specs/05-production-line.md), [docs/specs/00-domain-model.md](../docs/specs/00-domain-model.md)
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 4 에서 재고 부족 승인 시 `ProductionJob` 을 생성해 생산 큐에 삽입하는 지점까지 구현되어 있다. `ProductionJob` 타입은 Phase 1 에서 구현되어 있다.

## 목표

생산 큐 관리 및 생산 완료 트리거를 확정하고, 생산 완료 시 재고/주문 상태를 갱신한다.

## 구현 항목

### 1. 생산 완료 트리거 방식 확정

**기본 채택안: 수동 트리거.** 담당자가 생산 라인 메뉴에서 "생산 완료 처리" 명령을 실행하면, 큐의 선두 Job 하나를 완료 처리한다. (명세서가 제시한 대안: 즉시 완료 모의, 실시간/스케줄 기반 — 이번 구현에서는 채택하지 않는다.)

### 2. `ProductionQueue` (Model)

- FIFO 큐. `std::queue<ProductionJob>` 등으로 래핑.
- `enqueuedAt` 오름차순으로 처리 순서를 보장한다 (큐 삽입 순서 자체가 FIFO이므로, 삽입 시점 기록만 정확하면 별도 정렬 로직은 불필요).
- 인터페이스: 삽입(enqueue), 선두 확인/제거(peek/dequeue), 비어있는지 확인, 전체 목록 조회(대기 큐 뷰 출력용).
- **영속화 범위**: `ProductionQueue` 는 JSON 파일로 영속화하지 않는다 (인메모리 상태로만 유지). `SampleRepository`/`OrderRepository`(Phase 2/3)와 달리 이 phase 의 큐는 프로그램 종료 시 사라져도 무방한 런타임 작업 상태로 취급한다.

### 3. 생산 현황 표기 뷰

현재 생산 중인 Job(큐 선두)의 정보를 표시한다. 최소 표기 항목:
- 대상 주문 정보 (`orderId`, `sampleId`/시료명, `customerName`, 주문 수량)
- 실 생산량 (`actualProductionQuantity`)
- 총 생산 시간 (`totalProductionTime`)
- 진행 상황 표기 (`producedQuantity` 또는 진행률 — 표기 방식은 자유, 필수 요구사항 아님)

### 4. 대기 주문 확인 뷰

큐 전체를 FIFO 순서대로 출력한다. 표시 항목: `orderId`, `sampleId`/시료명, `shortageQuantity`, `actualProductionQuantity`, `totalProductionTime`, 큐 내 순번.

### 5. 생산 완료 처리 로직

**처리 흐름 (Job 완료 시)**
1. 큐 선두의 `ProductionJob` 을 꺼낸다 (FIFO).
2. 대상 `Sample` 의 `stockQuantity` 에 `actualProductionQuantity` 를 더한다.
3. 대상 `Order` 의 상태를 `PRODUCING` → `CONFIRMED` 로 전환한다.
4. Job 을 큐에서 제거하고 완료 처리 결과를 출력한다.

**엣지 케이스**
- 생산 큐가 비어 있는 상태에서 완료 처리 명령 실행 → "진행 중인 생산이 없습니다" 안내, 처리 거부.
- 여러 Job이 동일 시료를 대상으로 큐에 있는 경우에도 각 Job은 독립적으로 순서대로 처리한다 (Job 간 재고 합산/최적화는 하지 않는다).

## 참고: 계산 공식 (Phase 4에서 이미 적용됨, 재확인용)

```
actualProductionQuantity = ceil(shortageQuantity / yield)
totalProductionTime      = avgProductionTime * actualProductionQuantity
```

## 문서 동기화 작업

이 phase 완료 시 `docs/specs/05-production-line.md` 의 "2.4 생산 완료 처리 - 트리거" 절 잠정 문구를 "수동 트리거 채택 확정"으로 갱신한다.

## 테스트 관점 (RED 단계에서 작성할 대상)

- `ProductionQueue`
  - 여러 Job을 순서대로 삽입 후 dequeue 시 FIFO 순서(삽입 순서)가 보장됨을 확인.
  - 빈 큐에서 dequeue/peek 시도 시 적절히 처리됨을 확인 (예외 또는 실패 신호).
- 생산 완료 처리 로직 (GoogleMock 으로 `SampleRepository`/`OrderRepository` 를 목 처리)
  - 여러 건의 `PRODUCING` 주문이 등록 순서대로 완료 처리됨을 확인.
  - 완료 처리 후 대상 `Sample` 의 `stockQuantity` 가 `actualProductionQuantity` 만큼 증가함을 확인.
  - 완료 처리 후 대상 `Order` 의 상태가 `PRODUCING` → `CONFIRMED` 로 전환됨을 확인.
  - 빈 큐에서 완료 처리 명령 실행 시 거부되고 안내 메시지가 출력됨을 확인.

## 완료 기준 (Definition of Done)

- [ ] 여러 건의 `PRODUCING` 주문이 등록 순서대로 처리됨을 테스트로 확인한다.
- [ ] 생산 완료 후 `Sample` 재고와 `Order` 상태가 함께 갱신됨을 확인한다.
- [ ] 생산 라인 화면(현황/대기열)이 콘솔에서 올바르게 출력됨을 수동 검증한다.
