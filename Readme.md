## 1. Basic components overview

![Alt text](./.docs/TradingSystemSimpleArchDiagram.png)

*Architecture diagram ispired by book "Building Low Latency Applications in C++" by Sourav Ghosh*

**Market Data Publisher:** Broadcasts low-latency market data (ticks, order book updates) to downstream components.

**Matching Engine:** Core component responsible for ultra-low-latency order matching and order book state management.

**Order Gateway Server:** Handles inbound client orders, performs minimal validation, and forwards them to the Matching Engine.

**Market Data Consumer:** Consumes real-time market data streams for trading decisions and analytics.

**Order Gateway Encoder & Decoder:** Encodes and decodes high-performance binary messages between clients and the Order Gateway.

**Trading Engine:** Executes trading strategies and generates orders based on real-time market data.

## 2. Market Data publisher

![Alt text](./.docs/MarketDataPublisher.png)

*Architecture diagram ispired by book "Building Low Latency Applications in C++" by Sourav Ghosh*

**2.1. Market state changes**

Closed, Pre-open, Opening, Trading

**2.2. Instrument updates**

Information about instruments avariable for trading. Contains usually instrument metadata.

**2.3. Order Updates**

Publisher uses order update messages to communicate changes to the orders in the limit order book.
- Order Add: new passive order was added to the limit order book
- Order Modify: order was modified in price, quantity or both
- Order Delete: order was deleted from order book

**2.4. Trade Messages**

This kind of messages notifies market participant that match happend in the market.
Notification can consist of many Add, Modify, Delete to communicate about full and partiall fills.

**2.5. Market Statistics**

Exchange publish those to cummunicate different stats about instrument.


