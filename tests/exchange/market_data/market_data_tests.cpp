#include <gtest/gtest.h>
#include "market_update.h"

TEST(MarketUpdateTests, marketUpdateTypeToString_returns_proper_string)
{
    EXPECT_EQ("ADD", Exchange::marketUpdateTypeToString(Exchange::MarketUpdateType::ADD));
    EXPECT_EQ("MODIFY", Exchange::marketUpdateTypeToString(Exchange::MarketUpdateType::MODIFY));
    EXPECT_EQ("CANCEL", Exchange::marketUpdateTypeToString(Exchange::MarketUpdateType::CANCEL));
    EXPECT_EQ("TRADE", Exchange::marketUpdateTypeToString(Exchange::MarketUpdateType::TRADE));
    EXPECT_EQ("INVALID", Exchange::marketUpdateTypeToString(Exchange::MarketUpdateType::INVALID));
}

TEST(MarketUpdateTests, toString_returns_proper_string)
{
    Exchange::MEMarketUpdate market_update = { Exchange::MarketUpdateType::ADD, OrderId(1ul), TickerId(1), 
        Side::BUY, Price(1ul), Qty(1), Priority(1ul) };

    auto str = market_update.toString();

    EXPECT_EQ("MEMarketUpdate [type:ADD oid:1 ticker:1 side:BUY price:1 qty:1 priority:1]", str);
}