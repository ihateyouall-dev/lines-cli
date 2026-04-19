#include <client-utils/parsers.hpp>
#include <gtest/gtest.h>

TEST(Parsers, Date) {
    EXPECT_EQ(parse_date("1970.01.01"), Lines::Temporal::Date{Lines::Temporal::Days{0}});
    EXPECT_EQ(parse_date("1969.12.31"), Lines::Temporal::Date{Lines::Temporal::Days{-1}});

    EXPECT_THROW(parse_date("1970.13.01"), std::out_of_range);
    EXPECT_THROW(parse_date("1970.01.32"), std::out_of_range);
}

TEST(Parsers, Time) {
    EXPECT_EQ(parse_time("00:00:00"), Lines::Temporal::Timestamp{Lines::Temporal::Seconds{0}});
    EXPECT_EQ(parse_time("23:59:59"), Lines::Temporal::Timestamp{Lines::Temporal::Seconds{86399}});

    EXPECT_THROW(parse_time("24:00:00"), std::out_of_range);
    EXPECT_THROW(parse_time("00:60:00"), std::out_of_range);
    EXPECT_THROW(parse_time("00:00:60"), std::out_of_range);

}

TEST(Parsers, TimePoint) {
    EXPECT_EQ(parse_timepoint("1970.01.01"),
              Lines::Temporal::TimePoint{Lines::Temporal::Seconds{0}});
    EXPECT_EQ(parse_timepoint("1970.01.01_23:59:59"),
              Lines::Temporal::TimePoint{Lines::Temporal::Seconds{86399}});

    EXPECT_THROW(parse_timepoint("Garbage"), std::invalid_argument);
}
