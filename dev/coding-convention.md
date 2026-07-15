# C++ 코딩 컨벤션

이 문서는 "반도체 시료 생산주문관리 시스템" (SampleOrderSystem) 프로젝트의 C++ 코딩 컨벤션을 정의한다.
Claude Code는 이 프로젝트에서 코드를 작성/수정할 때 반드시 아래 규칙을 따른다.

## 기본 환경

- 언어 표준: C++20 (`stdcpp20`)
- 툴셋: MSVC v145 (Visual Studio)
- 문자셋: Unicode
- 테스트 프레임워크: GoogleTest / GoogleMock (`packages/gmock.1.11.0`)

## 파일 구성

- 헤더: `.h`, 구현: `.cpp`. 헤더/소스 파일명은 대표 클래스명과 동일하게 맞춘다
- MVC 구조를 따른다: `Model`, `View`, `Controller` 디렉터리(또는 네임스페이스)로 역할을 분리한다.
- 라이브러리는 `Lib` 디렉터리 아래에 관리한다.

## 네이밍 규칙

| 대상 | 규칙 | 예시 |
|---|---|---|
| 클래스 / 구조체 / enum / enum class | PascalCase | `OrderManager`, `OrderStatus` |
| 함수 / 메서드 | PascalCase | `RegisterSample()`, `ApproveOrder()` |
| 지역 변수 / 함수 매개변수 | camelCase | `orderId`, `sampleCount` |
| 멤버 변수 | camelCase + `_` 접미사 | `sampleName_`, `stockQuantity_` |
| 상수 / constexpr | PascalCase 또는 `k` 접두사 | `kMaxQueueSize` |
| 네임스페이스 | 소문자 snake_case | `order_system` |
| 파일명 | PascalCase (클래스명과 일치) | `ProductionLine.h` |

## 서식(Formatting)

- 들여쓰기: 스페이스 4칸 (탭 금지).
- 중괄호: Allman 스타일이 아닌 K&R 스타일 사용 (여는 중괄호를 같은 줄에 배치).
  ```cpp
  if (stock >= quantity) {
      Confirm(order);
  } else {
      EnqueueProduction(order);
  }
  ```
- 한 줄 최대 길이는 120자를 넘기지 않는다.
- 포인터/참조 기호는 타입에 붙인다: `Order* order`, `const std::string& name`.

## 커밋 및 변경 관리

- 하나의 커밋은 하나의 논리적 변경만 포함한다.
- 기능 요구사항에 없는 리팩터링, 추상화, 방어적 코드를 임의로 추가하지 않는다.
