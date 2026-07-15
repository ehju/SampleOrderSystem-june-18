# Phase 2 — 시료 관리 기능

> 참조 명세 원문: [docs/specs/02-sample-management.md](../docs/specs/02-sample-management.md), [docs/specs/00-domain-model.md](../docs/specs/00-domain-model.md)
> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.
> 전제: Phase 1 에서 `Sample` 클래스가 이미 구현되어 있다.

## 목표

시료(Sample) 등록/조회/검색 기능을 콘솔에서 End-to-End 로 동작하도록 완성한다. **등록된 시료만 이후 주문이 가능**하다는 것이 이 기능의 핵심 전제다.

## 구현 항목

### 1. `SampleRepository` (Model)

- 등록: `Sample` 추가. 등록 시 `stockQuantity = 0` 으로 초기화한다.
- 전체 조회: 등록된 모든 `Sample` 목록 반환.
- 이름 부분검색: 이름에 검색어가 포함된 `Sample` 목록 반환.
- ID 조회: `sampleId` 로 단건 조회 (주문 기능에서 존재 검증에 사용).
- 재고 갱신 인터페이스: `stockQuantity` 를 증가/감소시키는 메서드 (생산 완료/출고 기능에서 사용 예정, 이 phase 에서는 인터페이스만 있으면 됨).
- `sampleId` 는 자동 채번한다 (등록 시 사용자 입력 없음).
- 이름 중복 등록은 거부한다.

**JSON 파일 영속화 (PRD 4절)**
- `SampleRepository` 는 시료 데이터를 JSON 파일로 저장/로드한다.
- 시작 시: 지정된 JSON 파일이 존재하면 그 내용을 읽어 초기 상태를 구성한다. 파일이 없으면 빈 상태로 시작한다.
- 변경 시(등록, 재고 갱신): 변경 직후 JSON 파일에 즉시 반영한다 (지연 저장/배치 저장은 하지 않는다).
- JSON 직렬화/역직렬화 로직은 `Lib/` 아래의 JSON 유틸리티를 사용한다 (신규 구현 또는 기존 유틸리티 확장은 이 phase 범위 안에서 결정).
- 파일 경로/이름은 이 phase 에서 확정한다 (예: `Data/samples.json`).

### 2. `SampleController`

입력 검증 후 `SampleRepository` 에 위임한다.

**등록 검증 규칙**
| 항목 | 필수 | 검증 규칙 |
|---|---|---|
| 이름 | Y | 공백/빈 문자열 불가, 기존 등록된 이름과 중복 불가 |
| 평균 생산시간 | Y | 양수 |
| 수율 | Y | 0 초과 1 이하 실수 |

- 검증 실패 시 오류 사유를 출력하고 등록을 취소한다.

**엣지 케이스**
- 이름 중복 → "이미 존재하는 시료 이름입니다" 안내, 등록 거부.
- 수율 0 또는 음수 → 등록 거부.
- 평균 생산시간 0 또는 음수 → 등록 거부.

### 3. `SampleView`

- 등록 입력폼: 이름/평균 생산시간/수율 입력 receive.
- 목록/검색 결과 출력: `sampleId`, `name`, `avgProductionTime`, `yield`, `stockQuantity` 컬럼으로 표시.
- 등록된 시료가 없는 경우 "등록된 시료가 없습니다" 안내.
- 검색 결과가 없는 경우 "검색 결과가 없습니다" 안내.

## 테스트 관점 (RED 단계에서 작성할 대상)

- `SampleRepository`
  - 정상 등록 시 `sampleId` 자동 채번, `stockQuantity = 0` 초기화 확인.
  - 이름 중복 등록 시도 시 거부됨을 확인.
  - 이름 부분검색: 포함된 이름만 반환, 검색 결과 없을 때 빈 목록 반환.
  - ID 조회: 존재하는 ID / 존재하지 않는 ID 각각의 결과 확인.
  - JSON 파일 영속화: 등록/재고 갱신 후 JSON 파일에 반영되는지 확인. 기존 JSON 파일이 있는 상태로 Repository 를 생성하면 그 데이터가 로드됨을 확인.
- `SampleController`
  - 빈 이름/중복 이름/유효하지 않은 yield(0, 음수, 1 초과)/유효하지 않은 평균 생산시간(0, 음수) 각각에 대해 등록이 거부됨을 확인 (GoogleMock 으로 `SampleRepository` 를 목 처리하여 Controller 의 검증 로직만 격리 테스트).
  - 정상 입력 시 Repository 의 등록 메서드가 호출됨을 확인.

## 완료 기준 (Definition of Done)

- [ ] 콘솔에서 등록 → 조회 → 검색 3개 시나리오가 정상 동작한다.
- [ ] 이름 중복, 잘못된 yield/평균 생산시간 값에 대한 거부 테스트가 통과한다.
- [ ] `SampleRepository`/`SampleController` 단위 테스트가 통과한다.
- [ ] 시료 등록/재고 변경이 JSON 파일에 반영되고, Repository 재생성(프로그램 재시작에 준하는 시나리오) 후에도 데이터가 유지됨을 테스트로 확인한다.
