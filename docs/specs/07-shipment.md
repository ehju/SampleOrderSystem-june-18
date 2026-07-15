# 기능 명세: 출고 처리

> 관련 PRD: 3.7 출고 처리
> 도메인 모델: [00-domain-model.md](00-domain-model.md) - Order

## 1. 목적

재고가 충분히 확보된 `CONFIRMED` 상태 주문에 대해 실제 출고를 실행하고, 주문을 `RELEASE` 상태로 종료 처리한다.

## 2. 하위 기능

### 2.1 출고 대상 목록 조회

- `CONFIRMED` 상태의 주문만 필터링하여 표시한다.
- 표시 항목: `orderId`, `sampleId`/시료명, `customerName`, `quantity`

### 2.2 출고 실행

**입력 값**: 출고할 `orderId`

**처리 흐름**
1. 대상 주문이 존재하며 `CONFIRMED` 상태인지 확인
   - 존재하지 않거나 `CONFIRMED` 가 아니면 처리 거부 ("출고 가능한 주문이 아닙니다")
2. 대상 Sample 의 `stockQuantity` 에서 `quantity` 만큼 차감
   - 승인 단계(3.4)에서 재고가 충분한 경우 이미 차감되었는지, 아니면 출고 시점에 차감하는지는 정책에 따라 달라짐 (아래 "재고 차감 시점" 참조)
3. 주문 상태를 `RELEASE` 로 전환
4. 출고 완료 메시지 출력

### 2.3 재고 차감 시점 정책 (구현 시 확정 필요)

PRD 는 재고 차감이 "승인 시점"인지 "출고 시점"인지 명시하지 않는다. 두 가지 후보 정책을 제시한다.

| 정책 | 설명 | 장단점 |
|---|---|---|
| A. 승인 시 차감 (권장) | 3.4 승인에서 재고 충분 판정 시 즉시 `stockQuantity -= quantity` 수행. 출고 시에는 상태만 `RELEASE` 로 전환 | 재고가 이미 "배정"된 상태로 관리되어 이중 배정 방지가 쉬움. 모니터링(3.5)의 "재고량"이 실제 가용 재고를 정확히 반영 |
| B. 출고 시 차감 | 승인 시에는 상태만 전환하고, 출고 실행 시점에 `stockQuantity -= quantity` 수행 | `CONFIRMED` 상태에서도 재고가 실물 그대로 남아 있어 이중 배정 위험 있음 (다른 주문이 그 사이 승인되며 같은 재고를 또 사용할 수 있음) |

> 본 프로젝트는 **정책 A (승인 시 차감)** 를 기본안으로 채택한다. 이 경우 본 절의 "2.2 출고 실행" 단계 2번(재고 차감)은 생략하고, 상태 전환만 수행한다. 최종 결정은 구현 단계(Phase 4, [IMPLEMENTATION_PLAN.md](../IMPLEMENTATION_PLAN.md) 참조)에서 확정하고 04-order-approval.md / 07-shipment.md 를 동기화한다.

## 3. 엣지 케이스

- 이미 `RELEASE` 상태인 주문에 대해 재출고 시도 → 처리 거부
- `RESERVED`/`PRODUCING`/`REJECTED` 상태인 주문에 대해 출고 시도 → 처리 거부

## 4. 관련 컴포넌트 (MVC)

- **Model**: `Order`, `OrderRepository`, `Sample`, `SampleRepository` (정책 B 채택 시에만 재고 차감 필요)
- **View**: `ShipmentView` — 출고 대상 목록, 실행 결과 출력
- **Controller**: `ShipmentController`

## 5. 다른 기능과의 연관성

- **주문 승인(3.4)**: 재고 차감 시점 정책에 따라 이 기능과 책임이 나뉜다.
- **모니터링(3.5)**: 출고 완료 시 `CONFIRMED` 카운트 감소, `RELEASE` 카운트 증가로 즉시 반영되어야 한다.
