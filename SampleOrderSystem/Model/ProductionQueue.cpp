#include "ProductionQueue.h"

#include <stdexcept>

namespace order_system {

void ProductionQueue::Enqueue(const ProductionJob& job)
{
    jobs_.push_back(job);
}

ProductionJob ProductionQueue::Dequeue()
{
    if (jobs_.empty()) {
        throw std::out_of_range("ProductionQueue is empty");
    }

    ProductionJob frontJob = jobs_.front();
    jobs_.erase(jobs_.begin());
    return frontJob;
}

const ProductionJob& ProductionQueue::Peek() const
{
    if (jobs_.empty()) {
        throw std::out_of_range("ProductionQueue is empty");
    }

    return jobs_.front();
}

bool ProductionQueue::IsEmpty() const
{
    return jobs_.empty();
}

size_t ProductionQueue::Size() const
{
    return jobs_.size();
}

std::vector<ProductionJob> ProductionQueue::GetAll() const
{
    return jobs_;
}

}  // namespace order_system
