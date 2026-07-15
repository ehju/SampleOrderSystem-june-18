# Phase 9 — 테스트 보강 및 최종 QA

> 참조 명세 원문: `docs/specs/` 전체 (00-domain-model.md ~ 07-shipment.md), `docs/PRD.md`
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 0~8 이 모두 완료되어 전체 기능(시료 관리/주문 접수/승인·거절/생산 라인/모니터링/출고 처리/메인 메뉴)이 동작한다.

## 목표

전체 기능에 대한 통합 테스트를 보강하고, 구현 중 확정된 정책이 문서와 일치하는지 최종 점검한다.

## 구현 항목

### 1. 통합 테스트 시나리오 작성

Phase 8의 전체 시나리오를 자동화된 테스트로 전환 가능한 범위까지 작성한다.

**핵심 시나리오**: 시료 등록 → 주문 접수 → 승인(재고 충분 케이스) → 출고 → 모니터링 확인
**핵심 시나리오 2**: 시료 등록(낮은 재고) → 주문 접수(재고 초과 수량) → 승인(재고 부족 → PRODUCING 전환 확인) → 생산 완료 처리 → 상태가 CONFIRMED 로 전환됨 확인 → 출고 → 모니터링 확인
**핵심 시나리오 3**: 주문 접수 → 거절 → 모니터링에서 REJECTED 가 어떤 집계에도 포함되지 않음을 확인

### 2. 경계값/예외 케이스 커버리지 재점검

각 기능 명세서의 "엣지 케이스" 항목을 기준으로 체크리스트화하고 누락된 테스트를 보강한다.

| 기능 | 명세 문서 | 점검할 엣지 케이스 |
|---|---|---|
| 시료 관리 | [docs/specs/02-sample-management.md](../docs/specs/02-sample-management.md) | 이름 중복, yield 범위, avgProductionTime 범위, 검색 결과 없음 |
| 주문 접수 | [docs/specs/03-order-reservation.md](../docs/specs/03-order-reservation.md) | 존재하지 않는 sampleId, quantity < 1, 빈 customerName |
| 주문 승인/거절 | [docs/specs/04-order-approval.md](../docs/specs/04-order-approval.md) | 재고 충분/부족 분기, ceil 경계값, 이미 처리된 주문 재처리 |
| 생산 라인 | [docs/specs/05-production-line.md](../docs/specs/05-production-line.md) | FIFO 순서, 빈 큐 완료 처리 시도 |
| 모니터링 | [docs/specs/06-monitoring.md](../docs/specs/06-monitoring.md) | 재고==대기수량 경계, 재고==0/대기수량==0 |
| 출고 처리 | [docs/specs/07-shipment.md](../docs/specs/07-shipment.md) | CONFIRMED 아닌 주문 출고 시도, 재출고 시도 |
| 메인 메뉴 | [docs/specs/01-main-menu.md](../docs/specs/01-main-menu.md) | 숫자 외 입력, 범위 밖 메뉴, 초기 상태(시료 0건) |

### 3. 정책 문서 최종 검토

구현 중 확정된 다음 정책이 `docs/specs/` 문서와 일치하는지 확인하고, 불일치가 있으면 문서를 갱신한다.
- 재고 차감 시점: 정책 A (승인 시 차감) — `docs/specs/04-order-approval.md`, `docs/specs/07-shipment.md`
- 생산 완료 트리거 방식: 수동 트리거 — `docs/specs/05-production-line.md`
- sampleId 자동 채번 여부 — `docs/specs/02-sample-management.md`
- 재고 부족 시 기존 재고 활용 및 동시성 정책 — `docs/specs/04-order-approval.md`

### 4. 코딩 컨벤션 준수 여부 최종 점검

- 네이밍 규칙(PascalCase/camelCase/`_` 접미사 등) 준수 여부.
- K&R 중괄호 스타일, 4칸 들여쓰기, 120자 라인 제한 준수 여부.

## 테스트 관점 (RED 단계에서 작성할 대상)

- 위 "핵심 시나리오 1~3" 을 각각 통합 테스트(GoogleTest, 필요한 최소 범위에서만 GoogleMock 사용 — Repository 는 실제 구현을 사용하는 것을 권장)로 작성한다.
- "경계값/예외 케이스 재점검" 표의 항목 중 기존 phase 에서 테스트가 없었던 항목이 있다면 추가로 테스트를 작성한다 (신규 항목이 없다면 이 단계는 생략 가능).

## 완료 기준 (Definition of Done)

- [ ] 모든 기능 명세서의 엣지 케이스가 테스트로 커버된다.
- [ ] 문서(`docs/PRD.md`/`docs/specs/`)와 실제 구현의 정책 불일치가 없다.
- [ ] 전체 테스트 스위트(`build-and-test.ps1`)가 통과한다.
