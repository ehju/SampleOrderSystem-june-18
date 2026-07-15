# Phase 4 — 주문 승인/거절 기능 (재고 판정 & 정책 확정)

> 참조 명세 원문: [docs/specs/04-order-approval.md](../docs/specs/04-order-approval.md), [docs/specs/07-shipment.md](../docs/specs/07-shipment.md) 2.3절(재고 차감 시점), [docs/specs/00-domain-model.md](../docs/specs/00-domain-model.md)
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 3 에서 `OrderRepository`(생성/상태별 조회) 가, Phase 2 에서 `SampleRepository` 가 이미 구현되어 있다.

## 목표

승인/거절 유스케이스와 재고 판정 분기 로직을 완성한다. 재고 차감 시점 정책은 **정책 B: 출고 시 차감**으로 확정되어 있다 (승인 시에는 상태 전환만 수행하고, 실제 `stockQuantity -= quantity` 는 Phase 7 출고 실행 시점에 수행한다). 이 phase 에서는 승인 로직이 재고를 차감하지 않고 상태 전환만 수행하도록 구현한다.

## 구현 항목

### 1. 접수된 주문 목록 조회

- `RESERVED` 상태의 주문만 필터링하여 표시한다.
- 접수 순서(FIFO, `createdAt` 오름차순) 정렬을 권장한다.
- 표시 항목: `orderId`, `sampleId`(또는 시료명), `customerName`, `quantity`, `createdAt`.

### 2. 주문 승인 로직

**입력 값**: 승인할 `orderId`

**처리 흐름**
1. 대상 주문이 존재하며 `RESERVED` 상태인지 확인. 아니면 "승인 가능한 주문이 아닙니다" 안내, 처리 거부.
2. 대상 `Sample` 의 `stockQuantity` 조회.
3. **재고 충분** (`stockQuantity >= quantity`): 주문 상태를 `CONFIRMED` 로 전환한다. 정책 B에 따라 재고 차감은 하지 않는다 (실제 차감은 Phase 7 출고 시점에 수행).
4. **재고 부족** (`stockQuantity < quantity`): 아래 "생산 라인 자동 등록" 절차 수행 후 주문 상태를 `PRODUCING` 으로 전환.
5. 처리 결과 메시지 출력 (전환된 상태 포함).

**생산 라인 자동 등록 절차 (재고 부족 시)**
1. `shortageQuantity = quantity - stockQuantity` (재고가 0이어도 동일 공식 적용)
2. `actualProductionQuantity = ceil(shortageQuantity / yield)`
3. `totalProductionTime = avgProductionTime * actualProductionQuantity`
4. 위 값으로 `ProductionJob` 을 생성하여 생산 큐(FIFO)의 맨 뒤에 삽입한다 (큐 자체 구현은 Phase 5 담당이므로, 이 phase 에서는 `ProductionJob` 을 생성해 큐에 추가하는 지점까지만 구현한다).
5. 재고 차감은 이 시점에 하지 않는다 — 실제 차감/충족 처리는 생산 완료 시점(Phase 5)에 수행한다.

> **동시성/이중 배정 정책**: 재고 부족 시에도 기존 재고는 유지한 채 부족분(`shortageQuantity`)만큼만 생산하고, 생산 완료 시 "생산량"을 재고에 가산한 뒤 해당 주문을 `CONFIRMED` 로 전환한다 (Phase 5에서 처리). 이 phase 의 구현 범위는 단일 스레드 콘솔 애플리케이션이므로 동시 요청에 의한 이중 배정 문제는 발생하지 않는다 — 별도의 락/트랜잭션 처리는 요구사항에 없으므로 추가하지 않는다.

### 3. 주문 거절 로직

**입력 값**: 거절할 `orderId`

**처리 흐름**
1. 대상 주문이 존재하며 `RESERVED` 상태인지 확인.
2. 즉시 상태를 `REJECTED` 로 전환.
3. 재고/생산 큐에는 아무 영향을 주지 않는다.
4. 거절 완료 메시지 출력.

### 4. 상태 가드

- 이미 `RESERVED` 가 아닌 주문(즉 `REJECTED`/`CONFIRMED`/`PRODUCING`/`RELEASE`)에 대한 승인/거절 재시도는 거부하고 안내 메시지를 출력한다.

## 테스트 관점 (RED 단계에서 작성할 대상)

GoogleMock 으로 `SampleRepository`/`OrderRepository` 를 목 처리하여 승인/거절 로직을 격리 테스트한다.

- 재고 충분 분기: `stockQuantity >= quantity` 인 경우 주문이 `CONFIRMED` 로 전환됨을 확인. 재고 차감 메서드는 호출되지 않음을 확인 (`EXPECT_CALL(...).Times(0)` 등으로 검증).
- 재고 부족 분기: `stockQuantity < quantity` 인 경우 `ProductionJob` 이 생성되어 큐에 추가되고 주문이 `PRODUCING` 으로 전환됨을 확인. 이때도 재고는 차감되지 않음을 확인.
- `ceil` 계산 경계값: `shortageQuantity`/`yield` 가 나누어 떨어지는 경우와 떨어지지 않는 경우 각각의 `actualProductionQuantity` 값 확인 (예: `shortageQuantity=10, yield=0.9` → `ceil(11.11)=12`).
- 이미 처리된 주문(`REJECTED`/`CONFIRMED`/`PRODUCING`/`RELEASE`)에 대한 승인/거절 재시도가 거부됨을 확인.
- 거절 처리 시 상태가 `REJECTED` 로 전환되고 재고/큐에 영향이 없음을 확인.

## 완료 기준 (Definition of Done)

- [ ] 재고 충분/부족 분기 테스트가 모두 통과한다 (충분 분기에서도 재고가 차감되지 않고 상태만 전환됨을 포함).
- [ ] 재고 차감 정책(정책 B: 출고 시 차감)이 `docs/specs/04-order-approval.md`, `docs/specs/07-shipment.md` 와 코드에서 일치한다.
- [ ] 잘못된 상태의 주문에 대한 승인/거절 시도가 거부됨을 테스트로 확인한다.
