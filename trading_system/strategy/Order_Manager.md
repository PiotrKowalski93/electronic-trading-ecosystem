# Order Manager – Overview

The Order Manager is a core component responsible for handling the lifecycle of orders within the trading system. It acts as the central coordination layer between strategy, risk checks, and external execution venues.

## Purpose

The main purpose of the Order Manager is to ensure that all orders are:

- Validated before submission
- Properly tracked throughout their lifecycle
- Consistently synchronized with external systems (e.g., exchanges, gateways)

## Responsibilities

- Accept orders from upstream components (e.g., trading strategies)
- Perform pre-trade validation (basic checks, optional risk hooks)
- Assign and manage internal order identifiers
- Route orders to the appropriate execution gateway
- Process acknowledgments, fills, cancellations, and rejections
- Maintain in-memory state of all active and historical orders
- Ensure idempotency and consistency in event handling

## Key Concepts

- **Order Lifecycle Management:** New → Acknowledged → Partially Filled → Filled / Cancelled / Rejected
- **State Synchronization:** Keeps internal state aligned with external execution reports
- **Event-Driven Design:** Reacts to order events rather than polling
- **Low Latency Considerations:** Designed to minimize processing overhead and avoid blocking operations

## Key Takeaways

- The Order Manager is the **single source of truth** for order state
- It **decouples trading logic from execution details**
- It is critical for **correctness and consistency**, not just performance
- Must be **deterministic, thread-safe, and resilient to message duplication or reordering**