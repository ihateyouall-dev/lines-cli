#include "lines/temporal/clocks.hpp"
#include <client-utils/parsers.hpp>
#include <gtest/gtest.h>
#include <stdexcept>

TEST(Parsers, Date) {
    EXPECT_EQ(parse_date("1970.01.01"), Lines::Temporal::Date{Lines::Temporal::Days{0}});
    EXPECT_EQ(parse_date("1969.12.31"), Lines::Temporal::Date{Lines::Temporal::Days{-1}});

    EXPECT_THROW(parse_date("1970.13.01"), std::out_of_range);
    EXPECT_THROW(parse_date("1970.01.32"), std::out_of_range);

    EXPECT_THROW(parse_date(""), std::invalid_argument);
}

TEST(Parsers, RelativeDate) {
    EXPECT_EQ(parse_date("TODAY"), Lines::Temporal::LocalClock::today());
    EXPECT_EQ(parse_date("T"), Lines::Temporal::LocalClock::today());
    EXPECT_EQ(parse_date("tOdAy"), Lines::Temporal::LocalClock::today());
    EXPECT_EQ(parse_date("t"), Lines::Temporal::LocalClock::today());

    EXPECT_THROW(parse_date("TOD"), std::invalid_argument);
}

TEST(Parsers, DateOperators) {
    EXPECT_EQ(parse_date("1970.01.01+1d"), Lines::Temporal::Date{Lines::Temporal::Days{1}});
    EXPECT_EQ(parse_date("1970.01.01+2d"), Lines::Temporal::Date{Lines::Temporal::Days{2}});
    EXPECT_EQ(parse_date("1970.01.02+1d"), Lines::Temporal::Date{Lines::Temporal::Days{2}});

    EXPECT_EQ(parse_date("1970.01.02-1d"), Lines::Temporal::Date{Lines::Temporal::Days{0}});

    EXPECT_EQ(parse_date("1970.01.01+1d-2m+3y"),
              Lines::Temporal::Date{Lines::Temporal::Days{0}} + Lines::Temporal::Days{1} -
                  Lines::Temporal::Months{2} + Lines::Temporal::Years{3});

    EXPECT_EQ(parse_date("1970.01.01+123d"),
              Lines::Temporal::Date{Lines::Temporal::Days{0}} + Lines::Temporal::Days{123});

    EXPECT_EQ(parse_date("1970.01.01+0d-0m+0y"), Lines::Temporal::Date{Lines::Temporal::Days{0}});

    EXPECT_THROW(parse_date("1970.01.01/1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970.01+1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970.01.01.01+1d"), std::invalid_argument);

    EXPECT_THROW(parse_date("1970.01.01+1dd"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970.01.01++1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970.01.01+d"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970.01.01+1"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970.01.01+1-1d"), std::invalid_argument);

    EXPECT_THROW(parse_date("1970.01.01+1h-1m+1s"), std::invalid_argument);

    EXPECT_THROW(parse_date("1970.01.01abcgarbageetc"), std::invalid_argument);

    EXPECT_NO_THROW(parse_date("1970.01.01+65535d"));
    EXPECT_THROW(parse_date("1970.01.01+65536d"), std::out_of_range);
}

TEST(Parsers, RelativeDateOperators) {
    EXPECT_EQ(parse_date("TODAY+1d"),
              Lines::Temporal::LocalClock::today() + Lines::Temporal::Days{1});
    EXPECT_EQ(parse_date("TODAY+2d"),
              Lines::Temporal::LocalClock::today() + Lines::Temporal::Days{2});

    EXPECT_EQ(parse_date("T+1d"), Lines::Temporal::LocalClock::today() + Lines::Temporal::Days{1});

    EXPECT_EQ(parse_date("TODAY-1d"),
              Lines::Temporal::LocalClock::today() - Lines::Temporal::Days{1});

    EXPECT_EQ(parse_date("TODAY+1d-2m+3y"),
              Lines::Temporal::LocalClock::today() + Lines::Temporal::Days{1} -
                  Lines::Temporal::Months{2} + Lines::Temporal::Years{3});

    EXPECT_EQ(parse_date("TODAY+123d"),
              Lines::Temporal::LocalClock::today() + Lines::Temporal::Days{123});

    EXPECT_EQ(parse_date("TODAY+0d-0m+0y"), Lines::Temporal::LocalClock::today());

    EXPECT_THROW(parse_date("TODAY/1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("TOD+1d"), std::invalid_argument);

    EXPECT_THROW(parse_date("TODAY+1dd"), std::invalid_argument);
    EXPECT_THROW(parse_date("TODAY++1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("TODAY+d"), std::invalid_argument);
    EXPECT_THROW(parse_date("TODAY+1"), std::invalid_argument);
    EXPECT_THROW(parse_date("TODAY+1+1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("+1d"), std::invalid_argument);

    EXPECT_THROW(parse_date("TODAY+1h-1m+1s"), std::invalid_argument);

    EXPECT_THROW(parse_date("TODAYabcgarbageetc"), std::invalid_argument);

    EXPECT_NO_THROW(parse_date("TODAY+65535d"));
    EXPECT_THROW(parse_date("TODAY+65536d"), std::out_of_range);
}

TEST(Parsers, Time) {
    EXPECT_EQ(parse_time("00:00:00"), Lines::Temporal::Timestamp{Lines::Temporal::Seconds{0}});
    EXPECT_EQ(parse_time("23:59:59"), Lines::Temporal::Timestamp{Lines::Temporal::Seconds{86399}});

    EXPECT_THROW(parse_time("24:00:00"), std::out_of_range);
    EXPECT_THROW(parse_time("00:60:00"), std::out_of_range);
    EXPECT_THROW(parse_time("00:00:60"), std::out_of_range);
}

// TEST(Parsers, TimePoint) {
//     EXPECT_EQ(parse_timepoint("1970.01.01"),
//               Lines::Temporal::TimePoint{Lines::Temporal::Seconds{0}});
//     EXPECT_EQ(parse_timepoint("1970.01.01_23:59:59"),
//               Lines::Temporal::TimePoint{Lines::Temporal::Seconds{86399}});

//     EXPECT_THROW(parse_timepoint("Garbage"), std::invalid_argument);
// }
