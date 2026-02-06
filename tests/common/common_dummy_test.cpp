#include <gtest/gtest.h>

TEST(CommonDummyTest, AlwaysTrue)
{
    EXPECT_TRUE(true);
}

TEST(CommonDummyTest, SimpleEquality)
{
    int a = 42;
    int b = 40 + 2;
    EXPECT_EQ(a, b);
}
