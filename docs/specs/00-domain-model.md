# 도메인 모델 정의

> 모든 기능 명세서(`docs/specs/`)에서 공통으로 참조하는 핵심 엔티티와 값 정의.

## 1. Sample (시료)

| 속성 | 타입 | 설명 |
|---|---|---|
| sampleId | 정수 (고유값) | 시료 식별자. 등록 시 시스템이 자동 채번 |
| name | 문자열 | 시료 이름 |
| avgProductionTime | 정수 | 평균 생산시간. 1개 생산에 소요되는 평균 시간(분) |
| yield | 실수 (0.0 ~ 1.0) | 수율 = 정상 시료 수 / 총 생산 시료 수 |
| stockQuantity | 정수 (0 이상) | 현재 재고 수량. 생산 완료 시 증가, 출고 시 감소 |

**불변식**
- `name` 은 시스템 내에서 유일해야 한다 (중복 등록 방지).
- `yield` 는 0을 초과하고 1 이하여야 한다 (0이면 무한 생산이 되므로 등록 시 검증 필요).
- `stockQuantity` 는 음수가 될 수 없다.

## 2. Order (주문)

| 속성 | 타입 | 설명 |
|---|---|---|
| orderId | 정수 (고유값) | 주문 식별자 |
| sampleId | 정수 | 참조하는 시료 |
| customerName | 문자열 | 고객명 |
| quantity | 정수 (1 이상) | 주문 수량 |
| status | OrderStatus (enum) | 아래 참조 |
| createdAt | 시각 | 주문 접수 시각 (정렬/FIFO 판단에 사용 가능) |

### OrderStatus (enum class)

| 상태 | 의미 | 진입 조건 |
|---|---|---|
| RESERVED | 주문 접수 (초기 상태) | 시료 예약 시 |
| REJECTED | 주문 거절 (종료 상태, 모니터링 제외) | 담당자가 RESERVED 주문을 거절 |
| PRODUCING | 승인 완료, 재고 부족으로 생산 중 | 승인 시 재고 부족 판정 |
| CONFIRMED | 승인 완료, 출고 대기 중 | 승인 시 재고 충분 판정 또는 생산 완료 시 |
| RELEASE | 출고 완료 (종료 상태) | 출고 처리 실행 |

**상태 전이도**

```
RESERVED ──approve(재고충분)──► CONFIRMED ──ship──► RELEASE
RESERVED ──approve(재고부족)──► PRODUCING ──생산완료──► CONFIRMED ──ship──► RELEASE
RESERVED ──reject──► REJECTED
```

- `REJECTED`, `RELEASE` 는 종료 상태이며 이후 상태 변경이 없다.
- `REJECTED` 는 모든 모니터링/집계 대상에서 제외한다 (요구사항 3.5, 3.4).

## 3. ProductionJob (생산 작업 / 생산 큐 항목)

생산 라인이 처리하는 단위. 하나의 `PRODUCING` 주문은 하나의 ProductionJob 과 1:1로 대응한다고 가정한다.

| 속성 | 타입 | 설명 |
|---|---|---|
| orderId | 정수 | 대상 주문 참조 |
| sampleId | 정수 | 대상 시료 참조 |
| shortageQuantity | 정수 | 승인 시점의 재고 부족분 (주문 수량 - 가용 재고) |
| actualProductionQuantity | 정수 | 실 생산량 = `ceil(shortageQuantity / yield)` |
| totalProductionTime | 실수 | 총 생산 시간 = `avgProductionTime * actualProductionQuantity` |
| enqueuedAt | 시각 | 생산 큐 진입 시각 (FIFO 정렬 기준) |
| producedQuantity | 정수 | 현재까지 생산된 수량 (진행률 표기용, 자율 결정 가능) |

**계산 공식 (요구사항 3.6)**

```
actualProductionQuantity = ceil(shortageQuantity / yield)
totalProductionTime      = avgProductionTime * actualProductionQuantity
```

**큐 정책**
- FIFO (First-In-First-Out). `enqueuedAt` 오름차순으로 처리.
- 생산 라인은 "하나의 생산 라인 = 하나의 시료 생산 흐름"이므로, 동시 처리 가능한 Job 수는 구현 시 생산 라인 개수에 따라 달라질 수 있다 (PRD 2장 기준으로는 라인 수를 별도로 명시하지 않으므로, 초기 구현은 단일 라인 순차 처리로 가정하고 확장 지점을 열어둔다).

## 4. 파생 값 (모니터링에서 사용)

- **상태별 주문 수**: `RESERVED`, `CONFIRMED`, `PRODUCING`, `RELEASE` 각각의 카운트 (REJECTED 제외)
- **시료별 재고 상태 (요구사항 3.5)**
  - 고갈: `stockQuantity == 0`
  - 부족: `0 < stockQuantity < 해당 시료에 대해 대기 중인(RESERVED/PRODUCING) 주문 수량 합`
  - 여유: 그 외 (재고가 대기 주문 수량 합 이상)
