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
| 5 (병렬) | [phase5.md](phase5.md) | 생산 라인 기능 (FIFO 큐, 생산 완료 처리) | [05-production-line.md](../docs/specs/05-production-line.md) |
| 6 (병렬) | [phase6.md](phase6.md) | 모니터링 기능 | [06-monitoring.md](../docs/specs/06-monitoring.md) |
| 7 (병렬) | [phase7.md](phase7.md) | 출고 처리 기능 | [07-shipment.md](../docs/specs/07-shipment.md) |
| 8 | [phase8.md](phase8.md) | 메인 메뉴 통합 & 콘솔 UX 마감 (병렬 트랙 합류) | [01-main-menu.md](../docs/specs/01-main-menu.md) |
| 9 | [phase9.md](phase9.md) | 테스트 보강 및 최종 QA | 전체 |

## 진행 원칙

- Phase 는 기본적으로 순서대로 진행한다 (Phase N 은 Phase N-1 의 산출물에 의존). 단, **Phase 5/6/7 은 Phase 4 완료 후 서로 의존하지 않으므로 병렬로 진행**한다 (아래 의존 관계 참조). Phase 내부 세부 항목은 상황에 따라 재배치 가능하다.
- 각 Phase 종료 시 해당 phase{N}.md 의 완료 기준(Definition of Done)을 만족해야 다음 Phase(또는 병렬 트랙의 경우 Phase 8) 로 진행한다.
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
                       │
                       ├─ Phase 5 (생산 라인)   ─┐
                       ├─ Phase 6 (모니터링)     ─┼─ 병렬 진행 (상호 의존 없음)
                       └─ Phase 7 (출고 처리)   ─┘
                                      │
                                      └─ Phase 8 (메인 메뉴 통합, 병렬 트랙 합류 & 교차 검증)
                                           └─ Phase 9 (테스트 보강 & QA)
```

Phase 5/6/7은 Phase 4의 완료 기준(재고 차감 정책 확정, 재고 충분/부족 분기, `CONFIRMED`/`PRODUCING` 전환)이 충족되면 담당자 또는 서브에이전트를 나누어 동시에 착수한다. 각 phase{N}.md 의 완료 기준은 자체 단위 테스트만으로 검증하며, Phase 간 교차 시나리오(생산 완료 → 모니터링 반영, 출고 → 모니터링 반영)는 Phase 8에서 통합 검증한다.

## 확정된 정책

| 항목 | 관련 Phase | 확정안 |
|---|---|---|
| 재고 차감 시점 (승인 시 vs 출고 시) | Phase 4, 7 | **B안: 출고 시 차감**. 승인 시에는 상태 전환만 수행, 출고 실행 시점에 재고를 재확인 후 차감 (재고 부족 시 출고 거부) |
| 생산 완료 트리거 방식 | Phase 5 | 수동 트리거 ("생산 완료 처리" 명령) |
| sampleId 자동 채번 여부 | Phase 2 | 자동 채번 |
| 재고 부족 시 기존 재고 활용 및 동시성(이중 배정 방지) 정책 | Phase 4 | 부족분만 생산. 단일 사용자 순차 처리 콘솔 앱이므로 별도 예약/잠금 로직은 추가하지 않는다 |
| 데이터 저장 방식 (PRD 4절: "자료구조는 Json 사용") | Phase 2 | JSON 파일 영속화, 변경 즉시 저장 (Repository가 시작 시 로드, 변경 시 저장). 파일 경로/스키마 세부사항은 Phase 2에서 확정 |

> 위 정책은 모두 확정되었으며, 관련 `docs/specs/*.md`와 `plans/phase{N}.md`에 반영되어 있다.
