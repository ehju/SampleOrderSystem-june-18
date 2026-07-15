# Phase 7 — 출고 처리 기능

> 참조 명세 원문: [docs/specs/07-shipment.md](../docs/specs/07-shipment.md), [docs/specs/00-domain-model.md](../docs/specs/00-domain-model.md)
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 4 에서 재고 차감 시점 정책 **A (승인 시 차감)** 가 최종 확정되어 적용되어 있다. `OrderRepository`(상태별 조회) 는 Phase 3에서 구현되어 있다.

## 목표

`CONFIRMED` 주문의 출고를 실행하고 `RELEASE` 로 전환한다.

## 구현 항목

### 1. 출고 대상 목록 조회

- `CONFIRMED` 상태의 주문만 필터링하여 표시한다.
- 표시 항목: `orderId`, `sampleId`/시료명, `customerName`, `quantity`.

### 2. 출고 실행 로직

**입력 값**: 출고할 `orderId`

**처리 흐름**
1. 대상 주문이 존재하며 `CONFIRMED` 상태인지 확인. 아니면 "출고 가능한 주문이 아닙니다" 안내, 처리 거부.
2. **재고 차감은 수행하지 않는다** — Phase 4에서 정책 A(승인 시 차감)를 채택했으므로, 승인 시점에 이미 `stockQuantity` 가 차감되어 있다. 출고 단계에서는 상태 전환만 수행한다.
3. 주문 상태를 `RELEASE` 로 전환한다.
4. 출고 완료 메시지를 출력한다.

## 엣지 케이스

- 이미 `RELEASE` 상태인 주문에 대해 재출고 시도 → 처리 거부.
- `RESERVED`/`PRODUCING`/`REJECTED` 상태인 주문에 대해 출고 시도 → 처리 거부.

## 테스트 관점 (RED 단계에서 작성할 대상)

GoogleMock 으로 `OrderRepository`/`SampleRepository` 를 목 처리하여 출고 로직을 격리 테스트한다.

- `CONFIRMED` 가 아닌 주문(`RESERVED`/`PRODUCING`/`REJECTED`/`RELEASE`)에 대한 출고 시도가 거부됨을 확인.
- 정상 출고 시 주문 상태가 `RELEASE` 로 전환됨을 확인.
- 정책 A 적용에 따라 출고 시 `SampleRepository` 의 재고 차감 메서드가 호출되지 않음을 확인 (호출 여부를 GoogleMock 의 `EXPECT_CALL(...).Times(0)` 등으로 검증).

## 완료 기준 (Definition of Done)

- [ ] 출고 처리 후 모니터링 화면에서 `RELEASE` 카운트가 증가함을 확인한다 (Phase 6 완료 후 통합 검증, 이 phase 에서는 최소한 `OrderRepository` 조회로 상태 전환을 확인).
- [ ] 잘못된 상태의 주문에 대한 출고 시도가 거부됨을 확인한다.
