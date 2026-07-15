# 기능 명세: 시료 주문 (예약/접수)

> 관련 PRD: 3.3 시료 주문
> 도메인 모델: [00-domain-model.md](00-domain-model.md) - Order

## 1. 목적

고객이 원하는 시료와 수량을 주문 담당자가 시스템에 접수한다. 접수된 주문은 `RESERVED` 상태로 생성되며, 이후 승인/거절 처리(3.4)의 대상이 된다.

## 2. 입력 값

| 항목 | 필수 | 검증 규칙 |
|---|---|---|
| 시료 ID (sampleId) | Y | `SampleRepository` 에 등록된 시료여야 함 |
| 고객명 (customerName) | Y | 공백/빈 문자열 불가 |
| 주문 수량 (quantity) | Y | 1 이상의 정수 |

## 3. 처리 흐름

1. 입력값 검증
   - 시료 ID가 존재하지 않으면 등록 거부 ("존재하지 않는 시료입니다")
   - 고객명이 비어 있으면 등록 거부
   - 주문 수량이 1 미만이면 등록 거부 ("주문 수량은 1 이상이어야 합니다")
2. 검증 통과 시 Order 생성
   - `orderId` 자동 채번
   - `status = RESERVED`
   - `createdAt` = 현재 처리 시각 (FIFO 판단 및 접수 순서 표시에 사용)
3. 접수 완료 메시지와 생성된 주문 정보 출력

## 4. 엣지 케이스

- 재고 상태와 무관하게 예약 시점에는 재고 확인을 하지 않는다 (재고 판정은 승인 단계 3.4에서 수행).
- 동일 고객이 동일 시료를 중복 주문하는 것은 제한하지 않는다 (별도 주문으로 각각 생성).

## 5. 관련 컴포넌트 (MVC)

- **Model**: `Order`, `OrderStatus`, `OrderRepository` (생성/조회 인터페이스)
- **View**: `OrderReservationView` — 입력폼, 접수 결과 출력
- **Controller**: `OrderController` (주문 접수/승인/거절 공통 Controller 로 통합 가능) — `SampleRepository` 로 시료 유효성 검증 후 `OrderRepository` 에 생성 위임

## 6. 다른 기능과의 연관성

- **주문 승인/거절(3.4)**: 여기서 생성된 `RESERVED` 주문이 처리 대상 목록이 된다.
- **모니터링(3.5)**: 생성 직후 `RESERVED` 상태 카운트에 반영된다.
