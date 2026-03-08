## Market Data Recovery Model (Simplified)

This project implements a **simplified market data recovery model** intended for educational purposes.

In this implementation, when a **sequence gap** is detected in the incremental market data stream, the consumer **drops the current state and reconnects**, waiting for a new snapshot to rebuild the order book.

Simplified recovery flow:

```
Incremental feed
      |
      v
Sequence gap detected
      |
      v
Drop local state
Reconnect to market data feed
Wait for snapshot
Replay incremental messages
Resume live processing
```

### Production Systems

In real-world exchange market data feeds, recovery mechanisms are typically more sophisticated and include additional channels and protocols:

**1. Retransmission / Recovery Channel**

Instead of immediately requesting a full snapshot, the client requests only the **missing sequence range**.

Example:

```
expected_seq = 105
received_seq = 108

Missing messages: 106-107
```

Client sends a retransmission request:

```
Resend messages [106,107]
```

This allows the system to recover quickly without rebuilding the entire order book.

**2. Multiple Incremental Feed Lines (A/B/C)**

Exchanges often provide redundant multicast feeds:

```
Feed A
Feed B
Feed C
```

All feeds contain the same messages but may experience different packet loss patterns.
Trading systems perform **feed arbitration**, selecting the earliest valid message among the available feeds.

Example recovery strategy:

```
Gap detected on Feed A
↓
Check message availability on Feed B
↓
Use Feed B message instead of requesting retransmission
```

**3. Snapshot as Last Resort**

In production environments, a **full snapshot is usually the final fallback** when:

* retransmission fails
* the gap is too large
* recovery timeout occurs

### Why This Project Uses a Simplified Model

The goal of this project is to focus on:

* incremental feed processing
* sequence gap detection
* snapshot synchronization
* lock-free event pipeline

without introducing the additional complexity of retransmission protocols and feed arbitration.

These mechanisms could be added in future iterations to more closely resemble **production-grade market data infrastructure** used in electronic trading systems.