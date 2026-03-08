## System Architecture Overview

The project implements a simplified **electronic trading system pipeline**, focusing on low-latency market data processing and event-driven architecture.

### High-Level Data Flow

```text
            Network (Multicast Market Data)
                        │
                        ▼
              Market Data Consumer
          (incremental + snapshot sync)
                        │
                        ▼
                Order Book Engine
           (maintains instrument state)
                        │
                        ▼
             Lock-Free Event Queue
                (SPSC ring buffer)
                        │
                        ▼
                 Trading Engine
          (strategy / matching logic)
                        │
                        ▼
                Order Management
                (future extension)
```

### Components

#### Market Data Consumer

Responsible for receiving and synchronizing market data feeds.

Responsibilities:

* process incremental market updates
* synchronize with snapshot data
* detect sequence gaps
* resynchronize on feed inconsistencies
* publish normalized market events

#### Order Book Engine

Maintains the current state of the order book for each instrument.

Responsibilities:

* apply market data updates
* maintain bid/ask price levels
* provide consistent market state for trading logic

#### Lock-Free Event Queue

A **single-producer single-consumer (SPSC) ring buffer** used for communication between threads.

Goals:

* minimize latency
* avoid locks and contention
* provide deterministic event flow

#### Trading Engine

Consumes normalized market events and performs trading logic.

In this project it acts as a placeholder for:

* strategy logic
* order generation
* risk checks
* matching engine experimentation

### Threading Model

```text
MarketDataThread
       │
       ▼
Lock-Free Queue (SPSC)
       │
       ▼
TradingEngineThread
```

This design isolates **network I/O from trading logic**, reducing contention and improving latency predictability.

### Design Goals

* deterministic event processing
* low-latency message pipeline
* minimal synchronization overhead
* clear separation of concerns between components