# Phase 1 — 도메인 모델 구현 (Model 계층 기초)

> 참조 명세 원문: [docs/specs/00-domain-model.md](docs/specs/00-domain-model.md)
> 코딩 컨벤션: [dev/coding-convention.md](dev/coding-convention.md)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 0 에서 구성한 `Model/` 디렉터리와 GoogleTest/GoogleMock 테스트 프로젝트가 이미 존재한다.

## 목표

모든 기능이 공유하는 핵심 엔티티/값 타입을 `Model/` 아래에 구현한다. 이 phase 에서는 Repository/Controller/View 는 다루지 않는다 (엔티티 자체와 불변식 검증만).

## 구현 항목

### 1. `OrderStatus` (enum class)

값: `RESERVED`, `REJECTED`, `PRODUCING`, `CONFIRMED`, `RELEASE`

상태 의미:
| 상태 | 의미 |
|---|---|
| RESERVED | 주문 접수 (초기 상태) |
| REJECTED | 주문 거절 (종료 상태) |
| PRODUCING | 승인 완료, 재고 부족으로 생산 중 |
| CONFIRMED | 승인 완료, 출고 대기 중 |
| RELEASE | 출고 완료 (종료 상태) |

### 2. `Sample` 클래스

| 속성 | 타입 | 설명 |
|---|---|---|
| sampleId | 정수 | 시료 식별자 |
| name | 문자열 | 시료 이름 |
| avgProductionTime | 정수 | 평균 생산시간(분) |
| yield | 실수 (0 초과 1 이하) | 수율 |
| stockQuantity | 정수 (0 이상) | 현재 재고 수량 |

**불변식 (이 phase 에서 생성자로 검증할 항목)**
- `yield` 는 0을 초과하고 1 이하여야 한다.
- `stockQuantity` 는 음수가 될 수 없다.
- 이름 중복 검증은 Repository 책임이므로 이 phase 에서는 검증하지 않는다.

### 3. `Order` 클래스

| 속성 | 타입 | 설명 |
|---|---|---|
| orderId | 정수 | 주문 식별자 |
| sampleId | 정수 | 참조하는 시료 |
| customerName | 문자열 | 고객명 |
| quantity | 정수 (1 이상) | 주문 수량 |
| status | OrderStatus | 상태 |
| createdAt | 시각 | 주문 접수 시각 |

### 4. `ProductionJob` 클래스

| 속성 | 타입 | 설명 |
|---|---|---|
| orderId | 정수 | 대상 주문 참조 |
| sampleId | 정수 | 대상 시료 참조 |
| shortageQuantity | 정수 | 승인 시점의 재고 부족분 |
| actualProductionQuantity | 정수 | 실 생산량 = `ceil(shortageQuantity / yield)` |
| totalProductionTime | 실수 | 총 생산 시간 = `avgProductionTime * actualProductionQuantity` |
| enqueuedAt | 시각 | 생산 큐 진입 시각 (FIFO 정렬 기준) |
| producedQuantity | 정수 | 현재까지 생산된 수량 (진행률 표기용) |

이 phase 에서는 `ProductionJob` 의 생성자/getter 만 구현하면 된다. `ceil` 계산 로직 자체는 이 클래스가 값을 들고 있기만 하면 되고, 계산 수행은 Phase 4/5 의 승인/생산 로직에서 이루어진다.

## 테스트 관점 (RED 단계에서 작성할 대상)

GoogleTest 로 각 클래스에 대해 다음을 검증한다 (GoogleMock 목 객체는 이 phase 에서는 필요 없다 — 순수 값 타입이므로).

- `Sample`
  - 정상 값으로 생성 시 getter 가 입력값을 그대로 반환한다.
  - `yield` 가 0 이하이거나 1을 초과하면 생성이 거부된다 (예외 또는 실패 반환 — 컨벤션/기존 코드 스타일에 맞춰 결정).
  - `stockQuantity` 에 음수를 넣으면 생성이 거부된다.
- `Order`
  - 정상 값으로 생성 시 getter 가 입력값을 그대로 반환하며 `status` 초기값 확인.
- `ProductionJob`
  - 정상 값으로 생성 시 getter 가 입력값을 그대로 반환한다.
- `OrderStatus`
  - enum class 값 5개가 모두 정의되어 있고 비교 연산이 정상 동작한다.

## 완료 기준 (Definition of Done)

- [ ] `OrderStatus`, `Sample`, `Order`, `ProductionJob` 4개 타입이 컴파일되고 단위 테스트를 통과한다.
- [ ] `yield`/`stockQuantity` 불변식 검증 테스트가 포함되어 있다.
- [ ] 이름 중복 검증은 이 phase 범위가 아님을 테스트 스코프에서 명확히 한다 (Repository 책임, Phase 2에서 다룸).
