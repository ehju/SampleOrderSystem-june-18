# Phase 3 (refact) — 주문 접수(예약) 기능, Lib 활용 명시

> 참조 명세 원문: [docs/specs/03-order-reservation.md](../docs/specs/03-order-reservation.md), [docs/specs/00-domain-model.md](../docs/specs/00-domain-model.md)
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md)
> 이 문서는 [phase3.md](phase3.md)를 대체한다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 1 에서 `Order`/`OrderStatus` 가, Phase 2 에서 `SampleRepository` 가 이미 구현되어 있다. `Lib/PersistenceManager.h`, `Lib/Json.h` 는 이미 구현되어 있다.

## 목표

고객 주문을 `RESERVED` 상태로 접수하는 기능을 완성한다. 예약 시점에는 재고 확인을 하지 않는다 (재고 판정은 이후 승인 단계에서 수행).

## 구현 항목

### 0. 사전 작업: 기본 MVC(`Model`/`View`/`Controller`) 상속 가능하도록 리팩터링

현재 `SampleOrderSystem/Model/Model.h`, `View/View.h`, `Controller/Controller.h` 는 Phase 0 골격 단계에서 만든 투두리스트 예제용 **구체 클래스**다 (가상 함수 없음, 투두리스트 전용 데이터/메서드 보유). `OrderRepository`/`OrderController`/`OrderReservationView` 가 이를 상속받아 쓸 수 있도록, 아래 항목을 이 phase 안에서 먼저 리팩터링한다.

- `Model`
  - 투두리스트 전용 멤버(`m_items`, `AddItem`, `RemoveItem`, `GetItems`)는 제거한다.
  - 공통 기반 요소만 남긴다: `ChangeListener` 타입, `SetOnChanged(ChangeListener)`, `protected: void NotifyChanged() const`. 파생 클래스가 자신의 데이터(`OrderRepository` 의 경우 `Order` 목록)를 직접 소유하고, 변경 시 `NotifyChanged()`를 호출해 알린다.
  - 소멸자를 `virtual` 로 선언한다.
- `View`
  - 투두리스트 전용 메서드(`ShowMenu`, `ReadMenuOption`, `ReadItemText`, `ReadItemIndex`, `ShowItems`)는 제거한다.
  - 공통 요소(`ShowMessage`)만 남긴다. 도메인별 입출력(예: 주문 접수 입력폼/결과 출력)은 파생 클래스(`OrderReservationView`)에서 추가로 정의한다.
  - 소멸자를 `virtual` 로 선언한다.
- `Controller`
  - 생성자(`Model&`, `View&`)와 `Run()` 루프 골격은 공통으로 유지한다.
  - 투두리스트 전용 핸들러(`HandleAdd`, `HandleRemove`, `HandleShow`)는 제거한다.
  - `Run()` 내부에서 호출되는 처리 로직은 `protected virtual` 훅으로 선언하여 파생 클래스(`OrderController`)가 오버라이드하도록 한다.
  - 소멸자를 `virtual` 로 선언한다.

> 이 리팩터링은 Phase 3 의 원래 범위(주문 접수)를 넘어서는 기반 구조 변경이지만, 이후 모든 Order 관련 모듈이 이를 상속받아 쓰기 위한 전제 작업이므로 이 phase 에서 가장 먼저 수행한다. 기존 투두리스트 예제 동작에 의존하는 테스트/코드가 있다면 함께 정리한다.

### 1. `OrderRepository` (Model)

- `Model` 을 `public` 상속한다 (`class OrderRepository : public Model`). 변경(생성, 상태 전환) 시 `protected` `NotifyChanged()` 를 호출해 알린다.

- 생성: `Order` 추가. `orderId` 자동 채번, `status = RESERVED`, `createdAt` 은 현재 처리 시각으로 설정.
- 전체 조회.
- 상태별 조회 (예: `RESERVED` 목록 — Phase 4에서 사용 예정이지만 인터페이스는 이 phase 에서 마련).
- ID 조회.

**JSON 파일 영속화 (PRD 4절) — Lib 활용**
- `SampleRepository`(Phase 2)와 동일한 정책을 따른다: 시작 시 로드, 변경(생성 및 이후 phase 의 상태 변경) 시 즉시 반영.
- 파일 입출력은 이미 구현되어 있는 `Lib/PersistenceManager.h`(`PersistenceManager` 클래스)를 그대로 사용한다. 새로운 파일 저장/로드 유틸리티를 만들지 않는다.
  - 저장: `PersistenceManager::Save(key, jsonValue)`, key는 `"orders"` 로 고정한다 (`SampleRepository` 의 `"samples"` key와 별도 파일로 관리되며, 내부적으로 `Data/orders.json` 에 저장된다).
  - 로드: 시작 시 `PersistenceManager::TryLoad("orders", outData)` 로 시도하고, 실패(파일 없음/손상 복구 불가)하면 빈 상태로 시작한다.
- `Order` ↔ `json::JsonValue` 간 직렬화/역직렬화 변환 함수만 이 phase 에서 새로 작성한다. JSON 파싱/덤프 자체는 `Lib/Json.h`(`json::JsonValue`)의 `Parse`/`Dump`/`Find`/`operator[]` 등 기존 API를 사용하고, 별도의 JSON 파서/직렬화 로직을 새로 구현하지 않는다.

### 2. `OrderController` 예약 유스케이스

- `Controller` 를 `public` 상속한다 (`class OrderController : public Controller`). 기반 클래스의 `Run()` 루프를 그대로 쓰고, 예약 처리 로직은 기반 클래스가 제공하는 `protected virtual` 훅을 오버라이드하여 구현한다.

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

- `View` 를 `public` 상속한다 (`class OrderReservationView : public View`). 공통 `ShowMessage` 는 기반 클래스 것을 그대로 사용하고, 아래 도메인별 입출력 메서드를 파생 클래스에 추가한다.
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
  - JSON 파일 영속화: 주문 생성 후 `PersistenceManager` 를 통해 파일에 반영되는지 확인. 기존 데이터가 있는 상태로 Repository 를 생성하면 `PersistenceManager::TryLoad` 를 통해 그 데이터가 로드됨을 확인.

## 완료 기준 (Definition of Done)

- [ ] 등록된 시료에 대해서만 주문 접수가 가능함을 테스트로 확인한다.
- [ ] 접수된 주문이 `OrderRepository` 의 상태별 조회에서 `RESERVED` 로 집계됨을 테스트로 확인한다.
- [ ] 콘솔에서 주문 접수 시나리오가 정상 동작한다.
- [ ] 주문 생성이 `PersistenceManager` 를 통해 파일에 반영되고, Repository 재생성 후에도 데이터가 유지됨을 테스트로 확인한다.
