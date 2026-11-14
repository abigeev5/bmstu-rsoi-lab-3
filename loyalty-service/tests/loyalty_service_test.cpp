#include <gtest/gtest.h>

#include "domain/level.hpp"

namespace loyalty {

TEST(LevelFor, ThresholdsFromTheVariant) {
    EXPECT_EQ(LevelFor(0).status, "BRONZE");
    EXPECT_EQ(LevelFor(0).discount, 5);
    EXPECT_EQ(LevelFor(9).status, "BRONZE");

    EXPECT_EQ(LevelFor(10).status, "SILVER");
    EXPECT_EQ(LevelFor(10).discount, 7);
    EXPECT_EQ(LevelFor(19).status, "SILVER");

    EXPECT_EQ(LevelFor(20).status, "GOLD");
    EXPECT_EQ(LevelFor(20).discount, 10);
    EXPECT_EQ(LevelFor(25).status, "GOLD");
}

}
