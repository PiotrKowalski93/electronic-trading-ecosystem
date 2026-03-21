# Feature Engine – Quick Notes

## What is it?

A Feature Engine is a component that transforms raw market data into numerical signals (*features*) used by a trading strategy.

Market Data → Feature Engine → Strategy → Orders -> Order Gateway

Without it, the strategy operates on raw events.  
With it, the strategy operates on a structured representation of market state.

---

## Inputs

- Order Book (BBO or full depth)
- Trades (ticks)
- Market events (add / cancel / modify)

---

## Outputs (example features)

### Order Book Features
- Spread = ask - bid
- Mid price = (bid + ask) / 2
- Imbalance: (bid_qty - ask_qty) / (bid_qty + ask_qty)

---

### Order Flow Features
- Aggressive buy / sell volume
- Quantity ratio: buy_qty / (buy_qty + sell_qty)
- Order Flow Imbalance (OFI)

---

### Microstructure Features
- Price change velocity
- Event rate (events per second)
- Short-term trend

## Key Design Principles (HFT / Low Latency)

### 1. Incremental computation

Features are updated incrementally — never recomputed from scratch.

### 2. Performance critical

The Feature Engine runs on every market event, so it must be:

- allocation-free (no heap usage)
- cache-friendly (contiguous memory, simple structs)
- branch-efficient
- no virtual calls

---

### 3. Deterministic

- No randomness
- Same input → same output
- Required for backtesting vs live consistency

---

### 4. Time-awareness

Features should not accumulate forever.
Use rolling windows (e.g. last N ms / N events) or exponential decay.

## Aggressive Orders (important concept)

An aggressive order is an order that executes immediately against existing liquidity.

- Market orders → always aggressive
- Limit orders crossing the spread:
  - Buy ≥ best ask
  - Sell ≤ best bid

These orders **move the price** and represent real market pressure.

---

## Why Feature Engine matters

- Converts noisy market data into usable signals
- Captures real-time market dynamics
- Enables strategy decisions based on *state*, not raw events

Order flow often matters more than the visible order book.

---

## Minimal Example

```cpp
struct Features {
    double mid_price;
    double spread;
    double imbalance;
};

class FeatureEngine {
public:
    inline void onBookUpdate(const BookUpdate& u) noexcept {
        // incremental update logic
    }

    inline const Features& get() const noexcept {
        return features_;
    }

private:
    Features features_;
};
```

