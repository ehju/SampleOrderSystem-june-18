# Phase 0 — 프로젝트 기초 설정

> 코딩 컨벤션: [dev/coding-convention.md](../dev/coding-convention.md) (C++20, MVC, GoogleTest/GoogleMock)
> 이 문서는 자기완결적이다. 다른 phase 파일이나 PLAN.md를 참조하지 않고 이 문서만으로 작업한다.

## 목표

빌드 가능한 최소 골격 프로젝트를 구성한다. 이후 모든 phase 의 기반이 되므로, 이 phase 에서는 신규 기능 로직은 작성하지 않는다.

## 구현 항목

1. **Visual Studio 프로젝트/솔루션 구성**
   - C++20 (`stdcpp20`), 툴셋 MSVC v145, 문자셋 Unicode
2. **디렉터리 구조**
   - `Model/`, `View/`, `Controller/`, `Lib/` 4개 디렉터리를 리포지토리에 만든다 (컨벤션 문서의 MVC 구조).
3. **GoogleTest/GoogleMock 연동**
   - `packages/gmock.1.11.0` 패키지를 참조하는 테스트 전용 프로젝트를 메인 프로젝트와 분리하여 구성한다.
4. **`main()` 진입점 뼈대**
   - 콘솔 애플리케이션의 `main()` 함수를 작성한다. 이 단계에서는 빈 루프(또는 즉시 종료)만 있으면 된다 — 메뉴 라우팅은 Phase 8 에서 완성한다.
5. **빌드/테스트 스크립트**
   - 리포지토리 루트에 `build-and-test.ps1` 스크립트를 두어, 이후 모든 phase 의 RED/GREEN/REFACTOR 작업이 이 스크립트로 빌드와 테스트 실행을 확인할 수 있게 한다.

## 테스트 관점 (RED 단계에서 작성할 대상)

- 더미 테스트 1건: 예) `TEST(Bootstrap, DummyTestRuns) { EXPECT_TRUE(true); }` — 테스트 프로젝트가 정상적으로 빌드/실행되는지 확인하는 용도.
- 이 phase 는 기능 로직이 없으므로 별도의 GoogleMock 목(mock) 대상은 없다.

## 완료 기준 (Definition of Done)

- [ ] 솔루션이 빌드 성공한다.
- [ ] 테스트 프로젝트에서 더미 테스트 1건이 실행되고 통과한다.
- [ ] 디렉터리 구조(`Model/`, `View/`, `Controller/`, `Lib/`)가 컨벤션 문서와 일치한다.
- [ ] `build-and-test.ps1` 실행으로 빌드와 테스트가 모두 확인된다.

## 참고 (컨벤션 요약)

- 헤더 `.h` / 구현 `.cpp`, 파일명은 대표 클래스명과 동일.
- 네이밍: 클래스/함수 PascalCase, 지역변수/매개변수 camelCase, 멤버변수 camelCase + `_` 접미사, 상수 PascalCase 또는 `k` 접두사, 네임스페이스 snake_case.
- 서식: 4칸 스페이스 들여쓰기(탭 금지), K&R 중괄호 스타일, 한 줄 120자 이내, 포인터/참조 기호는 타입에 붙임.
- 요구사항에 없는 리팩터링/추상화/방어적 코드를 추가하지 않는다.
