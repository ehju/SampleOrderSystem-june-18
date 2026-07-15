#pragma once

namespace order_system {

// Order progress status.
enum class OrderStatus {
    RESERVED,   // Order received (initial state)
    REJECTED,   // Order rejected (terminal state)
    PRODUCING,  // Approved, producing due to stock shortage
    CONFIRMED,  // Approved, waiting for shipment
    RELEASE     // Shipment completed (terminal state)
};

}  // namespace order_system
