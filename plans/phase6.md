# Phase 6 — 모니터링 기능

> 참조 명세 원문: [docs/specs/06-monitoring.md](../docs/specs/06-monitoring.md), [docs/specs/00-domain-model.md](../docs/specs/00-domain-model.md) 4절(파생 값)
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 2~4 에서 `SampleRepository`, `OrderRepository` (생성/조회) 가 구현되어 있다. 별도의 캐시 없이 항상 Repository 를 조회하여 실시간 계산한다.
> 병렬 진행: 이 phase 는 Phase 5(생산 라인)/Phase 7(출고 처리) 와 동시에 진행 가능하다. 집계 로직은 GoogleMock 으로 Repository 데이터를 직접 세팅해 검증하므로, Phase 5/7의 실제 구현 완료 여부와 무관하게 이 phase 를 완결할 수 있다.

## 목표

상태별 주문 집계 및 재고 상태(여유/부족/고갈) 판정 기능을 완성한다.

## 구현 항목

### 1. `MonitoringService` (또는 Controller 내 로직)

**주문량 확인**
- 상태별(`RESERVED`/`CONFIRMED`/`PRODUCING`/`RELEASE`) 주문 개수를 계산한다.
- `REJECTED` 상태는 어떤 집계에도 포함하지 않는다.

**재고량 확인**
- 등록된 시료별로 현재 `stockQuantity` 를 표시한다.
- 각 시료에 대해 재고 상태를 판정한다.

**재고 상태 판정 규칙**
| 상태 | 조건 |
|---|---|
| 고갈 | `stockQuantity == 0` |
| 부족 | `stockQuantity > 0` 이고, 해당 시료의 `RESERVED`+`PRODUCING`+`CONFIRMED` 상태 주문의 `quantity` 합보다 `stockQuantity` 가 적음 |
| 여유 | 그 외 (재고가 대기 주문 수량 합 이상, 대기 주문이 없는 경우 포함) |

> 재고 차감 시점 정책은 **정책 B(출고 시 차감)** 로 확정되었다. 따라서 `CONFIRMED` 상태도 아직 재고가 차감되지 않았으므로(출고 시점에 차감), 대기 수량 합 계산에 포함한다 (`RELEASE`/`REJECTED` 는 제외).

### 2. `MonitoringView`

**주문 현황 표시 예시**
```
[주문 현황]
 RESERVED  : 3건
 PRODUCING : 2건
 CONFIRMED : 4건
 RELEASE   : 10건
```

**재고 현황 표시 예시**
```
[재고 현황]
 sampleId | name     | stockQuantity | 대기수량합 | 상태
    1     | WaferA   |      50       |     30     | 여유
    2     | WaferB   |       0       |     10     | 고갈
    3     | WaferC   |       5       |     20     | 부족
```

## 엣지 케이스

- 주문이 하나도 없는 경우: 모든 상태 카운트 0으로 표시.
- 시료가 하나도 없는 경우: 재고 현황 목록 비어 있음 안내.
- 재고 == 대기 주문 수량 합인 경계값: "여유"로 판정한다 (부족 조건은 `stockQuantity` 가 대기 수량 합보다 "적을" 때만 성립).
- 재고 0, 대기 수량도 0인 경우: 고갈 조건(`stockQuantity == 0`)이 우선 적용되어 "고갈"로 판정한다.

## 테스트 관점 (RED 단계에서 작성할 대상)

GoogleMock 으로 `SampleRepository`/`OrderRepository` 를 목 처리하여 집계 로직을 격리 테스트한다.

- 상태별 주문 카운트: `RESERVED`/`PRODUCING`/`CONFIRMED`/`RELEASE` 각각의 개수가 정확히 계산되고, `REJECTED` 는 제외됨을 확인.
- 재고 상태 판정 경계값
  - `stockQuantity == 대기수량합` → 여유.
  - `stockQuantity == 0` 이고 대기수량합 `== 0` → 고갈.
  - `stockQuantity == 0` 이고 대기수량합 `> 0` → 고갈.
  - `0 < stockQuantity < 대기수량합` → 부족.
  - `stockQuantity > 대기수량합` → 여유.
- 주문/시료가 하나도 없는 경우 각각의 출력이 올바름을 확인.

## 완료 기준 (Definition of Done)

- [ ] 여유/부족/고갈 3개 상태 각각을 재현하는 테스트 케이스가 통과한다.
- [ ] `RESERVED`/`PRODUCING`/`CONFIRMED`/`RELEASE` 상태별 카운트가 Repository 데이터(목 처리) 기준으로 정확히 계산됨을 테스트로 확인한다.
- [ ] Phase 3~5(접수/승인/생산/거절) 전체 시나리오 실행 후 모니터링 화면 수치가 즉시 올바르게 반영되는지는 Phase 8 통합 시나리오에서 최종 검증한다.
