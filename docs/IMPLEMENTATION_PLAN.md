# 구현 계획 (Implementation Plan)

> 대상 문서: [docs/PRD.md](PRD.md), 기능별 상세 명세는 [docs/specs/](specs/) 참조.
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md) (C++20, MVC, GoogleTest)

## 진행 원칙

- 각 Phase 는 이전 Phase 산출물에 의존한다. 순서대로 진행하되, Phase 내부의 세부 항목은 상황에 따라 재배치 가능하다.
- 각 Phase 종료 시 **완료 기준(Definition of Done)** 을 만족해야 다음 Phase 로 진행한다.
- 신규 기능 코드는 GoogleTest 기반 단위 테스트를 동반한다 (Phase 0 에서 프레임워크 셋업 후, 이후 모든 Phase 에서 테스트를 함께 작성).
- 요구사항에 없는 리팩터링/추상화/방어적 코드는 추가하지 않는다 (컨벤션 문서 준수).

---

## Phase 0 — 프로젝트 기초 설정

**목표**: 빌드 가능한 최소 골격 프로젝트 구성

- Visual Studio 프로젝트/솔루션 구성 (C++20, MSVC v145, Unicode)
- 디렉터리 구조 수립: `Model/`, `View/`, `Controller/`, `Lib/`
- GoogleTest/GoogleMock 패키지 연동 (`packages/gmock.1.11.0`) 및 테스트 프로젝트 분리
- `main()` 진입점 뼈대 작성 (빈 콘솔 루프)
- CI 또는 로컬에서 "빌드 성공 + 빈 테스트 스위트 실행 성공" 확인

**완료 기준**
- [ ] 솔루션 빌드 성공
- [ ] 테스트 프로젝트에서 더미 테스트 1건 실행 성공
- [ ] 디렉터리 구조가 컨벤션 문서와 일치

---

## Phase 1 — 도메인 모델 구현 (Model 계층 기초)

**참조 명세**: [specs/00-domain-model.md](specs/00-domain-model.md)

**목표**: 모든 기능이 공유하는 핵심 엔티티/값을 구현

- `OrderStatus` enum class 구현 (`RESERVED`, `REJECTED`, `PRODUCING`, `CONFIRMED`, `RELEASE`)
- `Sample` 클래스 구현 (sampleId, name, avgProductionTime, yield, stockQuantity)
- `Order` 클래스 구현 (orderId, sampleId, customerName, quantity, status, createdAt)
- `ProductionJob` 클래스 구현 (shortageQuantity, actualProductionQuantity, totalProductionTime, enqueuedAt, producedQuantity)
- 각 엔티티에 대한 생성자/getter 단위 테스트 작성 (불변식 검증 포함: yield 범위, stockQuantity 음수 방지 등)

**완료 기준**
- [ ] 4개 핵심 타입 컴파일 및 단위 테스트 통과
- [ ] 불변식(이름 중복은 Repository 책임이므로 제외, yield/stockQuantity 범위 등) 검증 테스트 포함

---

## Phase 2 — 시료 관리 기능

**참조 명세**: [specs/02-sample-management.md](specs/02-sample-management.md)

**목표**: 시료 등록/조회/검색 기능 완성 (End-to-End 콘솔 동작)

- `SampleRepository` 구현 (등록, 전체 조회, 이름 부분검색, ID 조회, 재고 갱신 인터페이스)
- `SampleController` 구현: 입력 검증(빈 이름, 중복 이름, yield/시간 범위) 후 Repository 위임
- `SampleView` 구현: 등록 입력폼, 목록/검색 결과 출력
- 단위 테스트: Repository 등록/중복거부/검색, Controller 검증 로직
- 콘솔에서 등록 → 조회 → 검색 흐름 수동 검증

**완료 기준**
- [ ] 시료 등록/조회/검색 3개 시나리오가 콘솔에서 정상 동작
- [ ] 이름 중복, 잘못된 yield/시간 값에 대한 거부 테스트 통과

---

## Phase 3 — 주문 접수(예약) 기능

**참조 명세**: [specs/03-order-reservation.md](specs/03-order-reservation.md)

**목표**: 고객 주문을 `RESERVED` 상태로 접수하는 기능 완성

- `OrderRepository` 구현 (생성, 전체/상태별 조회, ID 조회)
- `OrderController` 예약 유스케이스 구현: `sampleId` 존재 검증 → `Order` 생성
- `OrderReservationView` 구현: 입력폼, 접수 결과 출력
- 단위 테스트: 존재하지 않는 시료 거부, 수량 1 미만 거부, 정상 접수 시 상태 `RESERVED` 확인

**완료 기준**
- [ ] 등록된 시료에 대해서만 주문 접수 가능함을 확인
- [ ] 접수된 주문이 모니터링(Phase 6 이전이므로 Repository 조회로 임시 확인)에서 `RESERVED` 로 집계됨을 테스트로 확인

---

## Phase 4 — 주문 승인/거절 기능 (재고 판정 & 정책 확정)

**참조 명세**: [specs/04-order-approval.md](specs/04-order-approval.md), [specs/07-shipment.md](specs/07-shipment.md) 3절(재고 차감 시점)

**목표**: 승인/거절 유스케이스와 재고 판정 분기 로직 완성. **재고 차감 시점 정책(A/B)을 이 Phase 에서 최종 확정**하고 관련 명세 문서를 갱신한다.

- 접수된 주문(`RESERVED`) 목록 조회 기능 (FIFO 정렬)
- 주문 승인 로직
  - 재고 충분 → `CONFIRMED` 전환 (+ 정책 A 채택 시 재고 즉시 차감)
  - 재고 부족 → `shortageQuantity`, `actualProductionQuantity`, `totalProductionTime` 계산 후 `ProductionJob` 생성, `PRODUCING` 전환
- 주문 거절 로직 → `REJECTED` 전환
- 상태 가드: 이미 `RESERVED` 가 아닌 주문에 대한 재처리 방지
- 단위 테스트: 재고 충분/부족 각각의 분기, 이미 처리된 주문 재처리 거부, `ceil` 계산 경계값(나누어 떨어지는 경우/안 떨어지는 경우)

**완료 기준**
- [ ] 재고 충분/부족 분기 테스트 모두 통과
- [ ] 재고 차감 정책이 문서와 코드에서 일치
- [ ] 잘못된 상태의 주문에 대한 승인/거절 시도가 거부됨을 테스트로 확인

---

## Phase 5 — 생산 라인 기능 (FIFO 큐 & 생산 완료 처리)

**참조 명세**: [specs/05-production-line.md](specs/05-production-line.md)

**목표**: 생산 큐 관리 및 생산 완료 트리거 확정, 생산 완료 시 재고/주문 상태 갱신

- 생산 완료 트리거 방식 확정 (명세서 2.4절 제안 중 선택, 기본안: 수동 트리거 "다음 생산 완료 처리")
- `ProductionQueue` 구현 (FIFO, 예: `std::queue<ProductionJob>` 래핑)
- 생산 현황 표기 뷰 구현 (현재 생산 중 Job 정보)
- 대기 주문 확인 뷰 구현 (큐 전체 FIFO 순서 출력)
- 생산 완료 처리 로직: 큐 선두 Job dequeue → Sample 재고 가산 → 대상 Order `PRODUCING` → `CONFIRMED` 전환
- 단위 테스트: FIFO 순서 보장, 빈 큐에서 완료 처리 시도 시 거부, 완료 후 재고/상태 갱신 정합성

**완료 기준**
- [ ] 여러 건의 PRODUCING 주문이 등록 순서대로 처리됨을 테스트로 확인
- [ ] 생산 완료 후 Sample 재고와 Order 상태가 함께 갱신됨을 확인
- [ ] 생산 라인 화면에서 현황/대기열이 올바르게 출력됨을 수동 검증

---

## Phase 6 — 모니터링 기능

**참조 명세**: [specs/06-monitoring.md](specs/06-monitoring.md)

**목표**: 상태별 주문 집계 및 재고 상태(여유/부족/고갈) 판정 기능 완성

- `MonitoringService` (또는 Controller 내 로직): 상태별 주문 카운트 계산 (`REJECTED` 제외)
- 재고 상태 판정 로직: 시료별 `stockQuantity` vs 대기 주문(`RESERVED`+`PRODUCING`) 수량 합 비교 → 여유/부족/고갈
- `MonitoringView` 구현: 주문 현황, 재고 현황 출력
- 단위 테스트: 경계값(재고==대기수량, 재고==0인데 대기수량도 0인 경우 등)

**완료 기준**
- [ ] Phase 3~5에서 만든 시나리오(접수/승인/생산/거절)를 실행한 뒤 모니터링 화면 수치가 즉시 올바르게 반영됨을 확인
- [ ] 여유/부족/고갈 3개 상태 각각을 재현하는 테스트 케이스 통과

---

## Phase 7 — 출고 처리 기능

**참조 명세**: [specs/07-shipment.md](specs/07-shipment.md)

**목표**: `CONFIRMED` 주문의 출고 실행 및 `RELEASE` 전환

- 출고 대상(`CONFIRMED`) 목록 조회
- 출고 실행 로직: 상태 가드 → (정책 B 채택 시에만 재고 차감) → `RELEASE` 전환
- 단위 테스트: `CONFIRMED` 가 아닌 주문 출고 거부, 정상 출고 후 상태/재고 정합성

**완료 기준**
- [ ] 출고 처리 후 모니터링(Phase 6) 화면에서 `RELEASE` 카운트가 증가함을 확인
- [ ] 잘못된 상태의 주문에 대한 출고 시도가 거부됨을 확인

---

## Phase 8 — 메인 메뉴 통합 & 콘솔 UX 마감

**참조 명세**: [specs/01-main-menu.md](specs/01-main-menu.md)

**목표**: 모든 하위 기능을 메인 메뉴에서 라우팅하고, 전체 애플리케이션으로 통합

- `MainMenuView`/`MainMenuController`(또는 `AppController`) 구현: 5개 메뉴(시료 관리/주문/모니터링/출고 처리/생산 라인) + 종료
- 메인 메뉴 요약 정보(등록 시료 수, 총 재고 수량, 상태별 주문 수) 실시간 계산
- 잘못된 입력에 대한 공통 예외 처리 (숫자 외 입력, 범위 밖 메뉴 번호)
- 전체 시나리오 수동 시연: 시료 등록 → 주문 접수 → 승인(재고부족 케이스 유도) → 생산 완료 처리 → 출고 → 모니터링 확인

**완료 기준**
- [ ] 메인 메뉴에서 모든 하위 기능 진입/복귀가 정상 동작
- [ ] 전체 시나리오(등록→주문→승인→생산→출고→모니터링) 수동 시연 성공

---

## Phase 9 — 테스트 보강 및 최종 QA

**목표**: 전체 기능에 대한 통합 테스트 보강 및 문서 동기화

- 통합 테스트 시나리오 작성 (Phase 8의 전체 시나리오를 자동화된 테스트로 전환 가능한 범위까지)
- 경계값/예외 케이스 커버리지 재점검 (각 기능 명세서의 "엣지 케이스" 항목 기준 체크리스트화)
- 구현 중 확정된 정책(재고 차감 시점, 생산 완료 트리거 방식 등)이 `docs/specs/` 문서와 일치하는지 최종 검토 및 갱신
- 코딩 컨벤션 준수 여부 최종 점검 (네이밍, K&R 포맷, 120자 라인 제한 등)

**완료 기준**
- [ ] 모든 기능 명세서의 엣지 케이스가 테스트로 커버됨
- [ ] 문서(PRD/specs)와 실제 구현의 정책 불일치 없음
- [ ] 전체 테스트 스위트 통과

---

## Phase 간 의존 관계 요약

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

## 미확정 정책 목록 (구현 중 확정 필요)

| 항목 | 관련 Phase | 관련 명세 |
|---|---|---|
| 재고 차감 시점 (승인 시 vs 출고 시) | Phase 4 | [specs/04-order-approval.md](specs/04-order-approval.md), [specs/07-shipment.md](specs/07-shipment.md) |
| 생산 완료 트리거 방식 (수동/모의/실시간) | Phase 5 | [specs/05-production-line.md](specs/05-production-line.md) |
| sampleId 자동 채번 여부 | Phase 2 | [specs/02-sample-management.md](specs/02-sample-management.md) |
| 재고 부족 시 기존 재고 활용 및 동시성(이중 배정 방지) 정책 | Phase 4 | [specs/04-order-approval.md](specs/04-order-approval.md) |
