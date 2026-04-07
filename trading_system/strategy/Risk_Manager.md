# Risk Manager – Component Overview

## Purpose

The risk_manager component is responsible for enforcing trading constraints and protecting the system from invalid, excessive, or dangerous order flow. It acts as a pre-trade validation layer between the trading logic (strategy/order manager) and external execution venues.

Its primary goal is to ensure that **no order violating defined risk limits leaves the system**.

## Responsibilities

The component performs **real-time validation and monitoring** of orders and positions:

- Pre-trade checks
    - Max order size
    - Price sanity (e.g. fat-finger protection)
    - Instrument-specific constraints
    - Order rate limits (throttling)
- Position limits
    - Net position per instrument
    - Global exposure limits
    - Directional limits (long/short caps)
- PnL / Loss protection
    - Max daily loss
    - Strategy-level drawdown limits
    - Kill-switch triggers
- Credit / notional limits
    - Total notional exposure
    - Per-instrument notional caps
- Order flow control
    - Reject / modify / allow decisions
    - Optional queuing or throttling

## Role in the Architecture

Typical flow:

```Strategy → Order Manager → Risk Manager → Gateway / Exchange```

The risk_manager **sits in the critical path**, meaning:
- It must be low-latency
- It must be deterministic
- It must avoid allocations and locks where possible

## Failure Handling

If a risk rule is violated:
- Order is rejected (hard fail)
- Order is clipped/adjusted (soft enforcement)
- System triggers kill switch (halt trading)

All decisions should be:
- Logged (async if possible)
- Traceable (for post-trade analysis)

## Key Takeaways (for quick recall)
- Guardrail, not strategy logic
- Sits on the hot path → must be extremely fast
- Validates every order before it leaves the system
- Maintains real-time view of risk (positions, exposure, PnL)
- Can stop trading entirely if limits are breached