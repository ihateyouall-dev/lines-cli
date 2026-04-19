#include "lines/temporal/timepoint.hpp"
#include <client-utils/parsers.hpp>
#include <gtest/gtest.h>

TEST(Parsers, Timepoint) {
    EXPECT_EQ(parse_timepoint("1970.01.01"),
              Lines::Temporal::TimePoint{Lines::Temporal::Seconds{0}});
    EXPECT_EQ(parse_timepoint("1970.01.01_23:59:59"),
              Lines::Temporal::TimePoint{Lines::Temporal::Seconds{86399}});
    EXPECT_EQ(parse_timepoint("1969.12.31"), Lines::Temporal::TimePoint{Lines::Temporal::Days{-1}});

    EXPECT_THROW(parse_timepoint("1970.13.01"), std::out_of_range);
    EXPECT_THROW(parse_timepoint("1970.01.32"), std::out_of_range);
    EXPECT_THROW(parse_timepoint("1970.01.01_24:00:00"), std::out_of_range);
    EXPECT_THROW(parse_timepoint("1970.01.01_00:60:00"), std::out_of_range);
    EXPECT_THROW(parse_timepoint("1970.01.01_00:00:60"), std::out_of_range);

    EXPECT_THROW(parse_timepoint("Garbage"), std::invalid_argument);
}
