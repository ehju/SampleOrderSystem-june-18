#include <gtest/gtest.h>

#include <chrono>
#include <stdexcept>
#include <vector>

#include "../Model/ProductionQueue.h"

using order_system::ProductionJob;
using order_system::ProductionQueue;

namespace {

ProductionJob MakeJob(int orderId, int sampleId, int shortageQuantity, int actualProductionQuantity,
                       double totalProductionTime)
{
    return ProductionJob(orderId, sampleId, shortageQuantity, actualProductionQuantity, totalProductionTime,
                          std::chrono::system_clock::now(), 0);
}

}  // namespace

// 새로 생성된 큐는 비어 있다.
TEST(ProductionQueueTest, IsEmptyInitially)
{
    ProductionQueue queue;

    EXPECT_TRUE(queue.IsEmpty());
    EXPECT_EQ(queue.Size(), 0u);
}

// Enqueue 후에는 비어 있지 않다.
TEST(ProductionQueueTest, IsNotEmptyAfterEnqueue)
{
    ProductionQueue queue;
    queue.Enqueue(MakeJob(1, 10, 5, 6, 180.0));

    EXPECT_FALSE(queue.IsEmpty());
    EXPECT_EQ(queue.Size(), 1u);
}

// 여러 Job 을 삽입 순서대로 Dequeue 하면 FIFO 순서(삽입 순서)가 보장된다.
TEST(ProductionQueueTest, DequeuesInFifoOrder)
{
    ProductionQueue queue;
    queue.Enqueue(MakeJob(1, 10, 5, 6, 180.0));
    queue.Enqueue(MakeJob(2, 11, 3, 4, 120.0));
    queue.Enqueue(MakeJob(3, 12, 7, 8, 240.0));

    ProductionJob first = queue.Dequeue();
    ProductionJob second = queue.Dequeue();
    ProductionJob third = queue.Dequeue();

    EXPECT_EQ(first.GetOrderId(), 1);
    EXPECT_EQ(second.GetOrderId(), 2);
    EXPECT_EQ(third.GetOrderId(), 3);
    EXPECT_TRUE(queue.IsEmpty());
}

// Peek 은 선두 Job 을 제거하지 않고 조회만 한다.
TEST(ProductionQueueTest, PeekReturnsFrontJobWithoutRemovingIt)
{
    ProductionQueue queue;
    queue.Enqueue(MakeJob(1, 10, 5, 6, 180.0));
    queue.Enqueue(MakeJob(2, 11, 3, 4, 120.0));

    const ProductionJob& front = queue.Peek();

    EXPECT_EQ(front.GetOrderId(), 1);
    EXPECT_EQ(queue.Size(), 2u);
}

// 빈 큐에서 Dequeue 를 시도하면 예외를 던진다.
TEST(ProductionQueueTest, DequeueThrowsWhenQueueIsEmpty)
{
    ProductionQueue queue;

    EXPECT_THROW(queue.Dequeue(), std::out_of_range);
}

// 빈 큐에서 Peek 을 시도하면 예외를 던진다.
TEST(ProductionQueueTest, PeekThrowsWhenQueueIsEmpty)
{
    ProductionQueue queue;

    EXPECT_THROW(queue.Peek(), std::out_of_range);
}

// GetAll 은 큐에 쌓인 Job 전체를 FIFO 순서(삽입 순서)대로 반환하고, 큐 상태는 변경하지 않는다.
TEST(ProductionQueueTest, GetAllReturnsJobsInFifoOrderWithoutModifyingQueue)
{
    ProductionQueue queue;
    queue.Enqueue(MakeJob(1, 10, 5, 6, 180.0));
    queue.Enqueue(MakeJob(2, 11, 3, 4, 120.0));

    std::vector<ProductionJob> all = queue.GetAll();

    ASSERT_EQ(all.size(), 2u);
    EXPECT_EQ(all[0].GetOrderId(), 1);
    EXPECT_EQ(all[1].GetOrderId(), 2);
    EXPECT_EQ(queue.Size(), 2u);
}
