# 기능 명세: 주문 승인/거절

> 관련 PRD: 3.4 주문 승인/거절
> 도메인 모델: [00-domain-model.md](00-domain-model.md) - Order, ProductionJob

## 1. 목적

`RESERVED` 상태의 주문을 확인하고, 생산 라인 담당자가 승인 또는 거절을 결정한다. 승인 시 재고 상황에 따라 시스템이 자동으로 후속 상태를 결정한다.

## 2. 하위 기능

### 2.1 접수된 주문 목록 조회

- `RESERVED` 상태의 주문만 필터링하여 표시한다.
- 표시 항목: `orderId`, `sampleId`(또는 시료명), `customerName`, `quantity`, `createdAt`
- 접수 순서(FIFO, `createdAt` 오름차순) 정렬을 권장한다 (승인 처리 우선순위 판단에 도움).

### 2.2 주문 승인

**입력 값**: 승인할 `orderId`

**처리 흐름**
1. 대상 주문이 존재하며 `RESERVED` 상태인지 확인
   - 존재하지 않거나 `RESERVED` 가 아니면 처리 거부 ("승인 가능한 주문이 아닙니다")
2. 재고 판정
   - 대상 Sample 의 `stockQuantity` 조회
   - **재고 충분** (`stockQuantity >= quantity`): 주문 상태를 `CONFIRMED` 로 전환한다. **재고 차감은 이 시점에 수행하지 않는다** — 정책 B(출고 시 차감, [07-shipment.md](07-shipment.md) 참조)를 채택했으므로 실제 차감은 출고 처리 시점에 수행한다.
   - **재고 부족** (`stockQuantity < quantity`): 아래 "생산 라인 자동 등록" 절차 수행 후 주문 상태를 `PRODUCING` 으로 전환
3. 처리 결과 메시지 출력 (전환된 상태 포함)

**생산 라인 자동 등록 절차 (재고 부족 시)**
1. `shortageQuantity = quantity - stockQuantity` (재고가 0이어도 동일 공식 적용)
2. `actualProductionQuantity = ceil(shortageQuantity / yield)` (계산 상세는 [05-production-line.md](05-production-line.md) 참조)
3. `totalProductionTime = avgProductionTime * actualProductionQuantity`
4. 위 값으로 `ProductionJob` 을 생성하여 생산 큐(FIFO)의 맨 뒤에 삽입
5. 재고 차감은 이 시점에 하지 않는다 — 실제 차감/충족 처리는 생산 완료 시점([05-production-line.md](05-production-line.md))에 수행한다.

> **재고 일부 활용 여부 (확정)**: PRD 는 "재고가 부족한 경우 생산 라인에 자동 등록"만 명시하고, 기존 재고 일부를 우선 차감할지 여부는 명시하지 않는다. 아래와 같이 확정한다.
> - 재고 부족 시에도 기존 재고는 유지한 채 부족분(`shortageQuantity`)만큼만 생산하고, 생산 완료 시 "생산량 + 기존 재고"에서 주문 수량을 차감한다.
> - **동시성/이중 배정 정책**: 이 애플리케이션은 단일 사용자가 메뉴를 순차적으로 조작하는 콘솔 앱이므로, 여러 주문이 동시에 재고를 두고 경합하는 상황 자체가 발생하지 않는다. 따라서 별도의 예약(reservation)/잠금 로직은 추가하지 않는다.

### 2.3 주문 거절

**입력 값**: 거절할 `orderId`

**처리 흐름**
1. 대상 주문이 존재하며 `RESERVED` 상태인지 확인
2. 즉시 상태를 `REJECTED` 로 전환
3. 재고/생산 큐에는 아무 영향을 주지 않는다
4. 거절 완료 메시지 출력

**엣지 케이스**
- 이미 `REJECTED`/`CONFIRMED`/`PRODUCING`/`RELEASE` 상태인 주문에 대해 승인/거절 재시도 → 처리 거부 및 안내 메시지

## 3. 관련 컴포넌트 (MVC)

- **Model**: `Order`, `OrderRepository`, `Sample`, `SampleRepository`, `ProductionQueue`(또는 `ProductionLine`)
- **View**: `OrderApprovalView` — 접수 목록 표시, 승인/거절 입력 및 결과 출력
- **Controller**: `OrderController` — 재고 판정 로직을 포함한 승인/거절 유스케이스 처리 (Model 계층의 Service/UseCase 로 분리 권장: `OrderApprovalService`)

## 4. 다른 기능과의 연관성

- **생산 라인(3.6)**: 재고 부족 판정 시 여기서 생성된 `ProductionJob` 이 생산 큐에 등록되고, 생산 완료 시 주문 상태를 `PRODUCING` → `CONFIRMED` 로 전환하는 것은 생산 라인 기능이 담당한다.
- **모니터링(3.5)**: 승인/거절 처리 직후 상태별 카운트가 즉시 반영되어야 한다.
- **출고 처리(3.7)**: `CONFIRMED` 로 전환된 주문이 출고 처리 대상이 된다.
