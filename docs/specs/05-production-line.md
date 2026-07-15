# 기능 명세: 생산 라인

> 관련 PRD: 3.6 생산 라인
> 도메인 모델: [00-domain-model.md](00-domain-model.md) - ProductionJob

## 1. 목적

주문 승인(3.4) 시 재고 부족으로 생성된 생산 작업(ProductionJob)을 FIFO 큐로 관리하고, 생산 진행 상황 및 완료 처리를 담당한다.

## 2. 하위 기능

### 2.1 생산 현황 표기

- 현재 생산 중인 시료(맨 앞 Job, 혹은 동시 처리 중인 Job들)에 대한 정보를 표시한다.
- 최소 표기 항목 제안 (PRD 상 "자율 결정"이지만 최소 기준 제시):
  - 대상 주문 정보 (`orderId`, `sampleId`/시료명, `customerName`, 주문 수량)
  - 실 생산량 (`actualProductionQuantity`)
  - 총 생산 시간 (`totalProductionTime`)
  - 현재까지의 생산량/경과 시간 또는 진행률 (`producedQuantity` 혹은 `%`) — 표기 방식은 자유

### 2.2 대기 주문 확인

- 생산 큐에 대기 중인 `ProductionJob` 목록을 FIFO 순서(등록 순서, `enqueuedAt` 오름차순)대로 출력한다.
- 표시 항목: `orderId`, `sampleId`/시료명, `shortageQuantity`, `actualProductionQuantity`, `totalProductionTime`, 큐 내 순번

### 2.3 생산 계산 로직 (재확인)

```
actualProductionQuantity = ceil(shortageQuantity / yield)
totalProductionTime      = avgProductionTime * actualProductionQuantity
```

**예시**: `shortageQuantity = 10`, `yield = 0.9` → `actualProductionQuantity = ceil(10 / 0.9) = ceil(11.11) = 12`

### 2.4 생산 완료 처리

**트리거**: 콘솔 애플리케이션 특성상 "시간 경과"를 실시간으로 표현하기 어려우므로, 구현 시 아래 중 하나의 트리거 방식을 확정해야 한다 (PRD는 방식을 명시하지 않음).

- (권장) **수동 트리거**: 담당자가 생산 라인 메뉴에서 "생산 완료 처리" 또는 "다음 틱 진행" 명령을 실행하면, 큐의 선두 Job을 완료 처리
- (대안) **즉시 완료 모의(simulate)**: Job 등록과 동시에 총 생산 시간만 기록하고, 별도 명령 실행 시 시간 조건 충족 여부와 무관하게 완료 처리
- (대안) **실시간/스케줄 기반**: 실제 `totalProductionTime` 이 경과해야 완료 (콘솔 앱에서는 구현 복잡도가 높아 비권장)

> 구현 단계에서 위 방식 중 하나를 선택하고 본 문서를 갱신한다. 아래 처리 흐름은 트리거 방식과 무관하게 공통 적용된다.

**처리 흐름 (Job 완료 시)**
1. 큐 선두의 `ProductionJob` 을 꺼낸다 (FIFO)
2. 대상 Sample 의 `stockQuantity` 에 `actualProductionQuantity` 를 더한다
3. 대상 Order 의 `status` 를 `PRODUCING` → `CONFIRMED` 로 전환한다
4. Job 을 큐에서 제거하고 완료 처리 결과를 출력한다

**엣지 케이스**
- 생산 큐가 비어 있는 상태에서 완료 처리 명령 실행 → "진행 중인 생산이 없습니다" 안내
- 여러 Job이 동일 시료를 대상으로 큐에 있는 경우에도 각 Job은 독립적으로 순서대로 처리한다 (Job 간 재고 합산/최적화는 하지 않음 — 단순화)

## 3. 관련 컴포넌트 (MVC)

- **Model**: `ProductionJob`, `ProductionQueue`(FIFO 컨테이너, 예: `std::queue`/`std::deque` 래핑), `Sample`, `SampleRepository`, `Order`, `OrderRepository`
- **View**: `ProductionLineView` — 생산 현황, 대기열 출력
- **Controller**: `ProductionLineController` — 생산 완료 트리거 처리, Repository/Queue 갱신

## 4. 다른 기능과의 연관성

- **주문 승인(3.4)**: `PRODUCING` 전환 시 이 기능의 큐에 Job이 등록된다.
- **시료 관리(3.2)**: 생산 완료 시 `stockQuantity` 가 증가한다.
- **모니터링(3.5)**: 생산 큐 상태는 재고/주문 현황 집계에 간접적으로 영향을 준다 (완료 전까지는 `PRODUCING` 카운트에 남아있음).
