# 기능 명세: 모니터링

> 관련 PRD: 3.5 모니터링
> 도메인 모델: [00-domain-model.md](00-domain-model.md) - 파생 값

## 1. 목적

담당자가 현재 시스템 상태(주문 현황, 재고 현황)를 한눈에 파악할 수 있도록 집계 정보를 제공한다.

## 2. 하위 기능

### 2.1 주문량 확인

- 상태별(`RESERVED` / `CONFIRMED` / `PRODUCING` / `RELEASE`) 주문 목록 및 개수를 표시한다.
- `REJECTED` 상태는 유효한 주문이 아니므로 어떤 집계에도 포함하지 않는다.

**표시 예시**

```
[주문 현황]
 RESERVED  : 3건
 PRODUCING : 2건
 CONFIRMED : 4건
 RELEASE   : 10건
```

- 각 상태를 선택하면 해당 상태의 상세 주문 목록(주문ID/시료/고객명/수량)을 조회할 수 있도록 구성 권장 (필수 아님, 최소 요건은 개수 확인).

### 2.2 재고량 확인

- 등록된 시료별로 현재 재고 수량(`stockQuantity`)을 표시한다.
- 각 시료에 대해 "주문 대비 재고 수량"에 따른 상태를 함께 표기한다.

**재고 상태 판정 규칙**

| 상태 | 조건 |
|---|---|
| 고갈 | `stockQuantity == 0` |
| 부족 | `stockQuantity > 0` 이고, 해당 시료의 미출고 대기 주문 수량 합(`RESERVED` + `PRODUCING` + `CONFIRMED` 상태 주문의 `quantity` 합) 보다 `stockQuantity` 가 적음 |
| 여유 | 그 외 (재고가 대기 주문 수량 합 이상, 대기 주문이 없는 경우 포함) |

> **비고 (확정)**: 재고 차감 시점 정책은 **B안(출고 시 차감)** 으로 확정되었다([07-shipment.md](07-shipment.md) 참조). 즉 `CONFIRMED` 상태에서도 재고는 아직 차감되지 않고 `RELEASE` 로 전환되는 순간(출고 실행 시점)에 차감된다. 따라서 "주문 대비" 기준은 아직 출고되지 않은 모든 상태(`RESERVED` + `PRODUCING` + `CONFIRMED`)의 수량 합으로 계산한다 (`RELEASE`/`REJECTED` 는 제외).

**표시 예시**

```
[재고 현황]
 sampleId | name     | stockQuantity | 대기수량합 | 상태
    1     | WaferA   |      50       |     30     | 여유
    2     | WaferB   |       0       |     10     | 고갈
    3     | WaferC   |       5       |     20     | 부족
```

## 3. 엣지 케이스

- 주문이 하나도 없는 경우: 모든 상태 카운트 0으로 표시
- 시료가 하나도 없는 경우: 재고 현황 목록 비어 있음 안내

## 4. 관련 컴포넌트 (MVC)

- **Model**: `OrderRepository`(상태별 조회/카운트), `SampleRepository`(재고 조회), 집계 로직은 `MonitoringService` 로 분리 권장
- **View**: `MonitoringView` — 주문 현황/재고 현황 출력
- **Controller**: `MonitoringController`

## 5. 다른 기능과의 연관성

- 모든 상태 변경 기능(주문 접수/승인/거절/생산 완료/출고)이 실행된 직후 즉시 반영되어야 하므로, 별도의 캐시 없이 항상 Repository 를 조회하여 실시간 계산하는 것을 권장한다.
