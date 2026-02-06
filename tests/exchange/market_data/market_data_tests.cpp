#include <gtest/gtest.h>
#include "market_update.h"

TEST(MarketUpdateTests, marketUpdateTypeToString_returns_proper_string_ADD){
    auto str = Exchange::marketUpdateTypeToString(Exchange::MarketUpdateType::ADD);

    EXPECT_EQ("ADD", str);
}

TEST(MarketUpdateTests, toString_returns_proper_string){
    MEMarketUpdate market_update = {Exchange::MarketUpdateType::ADD, OrderId}

    auto str = Exchange::marketUpdateTypeToString(Exchange::MarketUpdateType::ADD);

    EXPECT_EQ("ADD", str);
}