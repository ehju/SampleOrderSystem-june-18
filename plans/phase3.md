# Phase 3 — 주문 접수(예약) 기능

> 참조 명세 원문: [docs/specs/03-order-reservation.md](docs/specs/03-order-reservation.md), [docs/specs/00-domain-model.md](docs/specs/00-domain-model.md)
> 코딩 컨벤션: [dev/coding-convention.md](dev/coding-convention.md)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 1 에서 `Order`/`OrderStatus` 가, Phase 2 에서 `SampleRepository` 가 이미 구현되어 있다.

## 목표

고객 주문을 `RESERVED` 상태로 접수하는 기능을 완성한다. 예약 시점에는 재고 확인을 하지 않는다 (재고 판정은 이후 승인 단계에서 수행).

## 구현 항목

### 1. `OrderRepository` (Model)

- 생성: `Order` 추가. `orderId` 자동 채번, `status = RESERVED`, `createdAt` 은 현재 처리 시각으로 설정.
- 전체 조회.
- 상태별 조회 (예: `RESERVED` 목록 — Phase 4에서 사용 예정이지만 인터페이스는 이 phase 에서 마련).
- ID 조회.

### 2. `OrderController` 예약 유스케이스

**입력 값**
| 항목 | 필수 | 검증 규칙 |
|---|---|---|
| 시료 ID (sampleId) | Y | `SampleRepository` 에 등록된 시료여야 함 |
| 고객명 (customerName) | Y | 공백/빈 문자열 불가 |
| 주문 수량 (quantity) | Y | 1 이상의 정수 |

**처리 흐름**
1. `sampleId` 가 `SampleRepository` 에 존재하는지 확인 → 없으면 "존재하지 않는 시료입니다" 안내, 등록 거부.
2. `customerName` 이 비어 있으면 등록 거부.
3. `quantity` 가 1 미만이면 "주문 수량은 1 이상이어야 합니다" 안내, 등록 거부.
4. 검증 통과 시 `OrderRepository` 에 `Order` 생성 위임 (`status = RESERVED`).
5. 접수 완료 메시지와 생성된 주문 정보 출력.

**엣지 케이스**
- 동일 고객이 동일 시료를 중복 주문하는 것은 제한하지 않는다 (별도 주문으로 각각 생성).

### 3. `OrderReservationView`

- 입력폼: 시료 ID, 고객명, 주문 수량 입력.
- 접수 결과 출력: 생성된 주문 정보(orderId 포함).

## 테스트 관점 (RED 단계에서 작성할 대상)

- `OrderController` (GoogleMock 으로 `SampleRepository`/`OrderRepository` 를 목 처리하여 격리 테스트)
  - 존재하지 않는 `sampleId` 로 주문 시도 → 거부됨을 확인.
  - `quantity` 가 1 미만(0, 음수)인 경우 거부됨을 확인.
  - `customerName` 이 빈 문자열인 경우 거부됨을 확인.
  - 정상 입력 시 `OrderRepository` 의 생성 메서드가 `status = RESERVED` 로 호출됨을 확인.
- `OrderRepository`
  - 정상 생성 시 `orderId` 자동 채번, `status = RESERVED` 확인.
  - 상태별 조회 시 `RESERVED` 주문만 반환됨을 확인 (다른 상태 주문이 섞여 있는 상황을 가정한 테스트 데이터로 검증).

## 완료 기준 (Definition of Done)

- [ ] 등록된 시료에 대해서만 주문 접수가 가능함을 테스트로 확인한다.
- [ ] 접수된 주문이 `OrderRepository` 의 상태별 조회에서 `RESERVED` 로 집계됨을 테스트로 확인한다.
- [ ] 콘솔에서 주문 접수 시나리오가 정상 동작한다.
