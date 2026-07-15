# PLAN — SampleOrderSystem 구현 계획 (Phase 0~9)

> 원본: [docs/IMPLEMENTATION_PLAN.md](../docs/IMPLEMENTATION_PLAN.md), 기능별 상세 명세: [docs/specs/](../docs/specs/)
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md) (C++20, MVC, GoogleTest/GoogleMock)
> 각 phase의 세부 구현 계획은 `phase{N}.md` (N=0~9) 참조. 각 phase 파일은 자기완결적이며, 담당 RED/GREEN/REFACTOR 작업자는 해당 phase 파일과 현재 코드만 보고 작업한다.

## Phase 목록

| Phase | 파일 | 목표 | 참조 명세 |
|---|---|---|---|
| 0 | [phase0.md](phase0.md) | 프로젝트 기초 설정 (빌드 가능한 최소 골격) | - |
| 1 | [phase1.md](phase1.md) | 도메인 모델 구현 (OrderStatus/Sample/Order/ProductionJob) | [00-domain-model.md](../docs/specs/00-domain-model.md) |
| 2 | [phase2.md](phase2.md) | 시료 관리 기능 (등록/조회/검색) | [02-sample-management.md](../docs/specs/02-sample-management.md) |
| 3 | [phase3.md](phase3.md) | 주문 접수(예약) 기능 | [03-order-reservation.md](../docs/specs/03-order-reservation.md) |
| 4 | [phase4.md](phase4.md) | 주문 승인/거절 기능 (재고 판정, 재고 차감 정책 확정) | [04-order-approval.md](../docs/specs/04-order-approval.md), [07-shipment.md](../docs/specs/07-shipment.md) |
| 5 | [phase5.md](phase5.md) | 생산 라인 기능 (FIFO 큐, 생산 완료 처리) | [05-production-line.md](../docs/specs/05-production-line.md) |
| 6 | [phase6.md](phase6.md) | 모니터링 기능 | [06-monitoring.md](../docs/specs/06-monitoring.md) |
| 7 | [phase7.md](phase7.md) | 출고 처리 기능 | [07-shipment.md](../docs/specs/07-shipment.md) |
| 8 | [phase8.md](phase8.md) | 메인 메뉴 통합 & 콘솔 UX 마감 | [01-main-menu.md](../docs/specs/01-main-menu.md) |
| 9 | [phase9.md](phase9.md) | 테스트 보강 및 최종 QA | 전체 |

## 진행 원칙

- Phase 는 순서대로 진행한다 (Phase N 은 Phase N-1 의 산출물에 의존). Phase 내부 세부 항목은 상황에 따라 재배치 가능하다.
- 각 Phase 종료 시 해당 phase{N}.md 의 완료 기준(Definition of Done)을 만족해야 다음 Phase 로 진행한다.
- 신규 기능 코드는 GoogleTest/GoogleMock 기반 단위 테스트를 동반한다.
- 요구사항에 없는 리팩터링/추상화/방어적 코드는 추가하지 않는다 (코딩 컨벤션 준수).
- 빌드/테스트는 리포지토리 루트의 `build-and-test.ps1` 스크립트로 수행한다.

## Phase 간 의존 관계

```
Phase 0 (기초 설정)
   └─ Phase 1 (도메인 모델)
        └─ Phase 2 (시료 관리)
             └─ Phase 3 (주문 접수)
                  └─ Phase 4 (주문 승인/거절) ── 재고 차감 정책 확정
                       └─ Phase 5 (생산 라인)
                            └─ Phase 6 (모니터링)
                                 └─ Phase 7 (출고 처리)
                                      └─ Phase 8 (메인 메뉴 통합)
                                           └─ Phase 9 (테스트 보강 & QA)
```

## 미확정 정책 (구현 중 확정 필요)

| 항목 | 관련 Phase | 기본 채택안 |
|---|---|---|
| 재고 차감 시점 (승인 시 vs 출고 시) | Phase 4 | A안: 승인 시 차감 |
| 생산 완료 트리거 방식 | Phase 5 | 수동 트리거 ("생산 완료 처리" 명령) |
| sampleId 자동 채번 여부 | Phase 2 | 자동 채번 |
| 재고 부족 시 기존 재고 활용 및 동시성(이중 배정 방지) 정책 | Phase 4 | 부족분만 생산, 동시성 정책은 Phase 4에서 최종 확정 |
