#include "client-utils/parsers.hpp"
#include "lines/tasks/task_repeat.hpp"
#include "lines/temporal/clocks.hpp"
#include "lines/temporal/datetime.hpp"
#include "lines/temporal/duration.hpp"
#include "lines/temporal/timepoint.hpp"
#include "lines/temporal/timestamp.hpp"
#include "lines/temporal/ymd.hpp"

#include "gtest/gtest.h"
#include <cstddef>
#include <stdexcept>

using namespace Lines::Temporal;
using namespace Literals;

TEST(Parsers, Date) {
    EXPECT_EQ(parse_date("1970/01/01"), Date{0_d});
    EXPECT_EQ(parse_date("1969/12/31"), Date{-1_d});

    EXPECT_THROW(parse_date("1970/13/01"), std::out_of_range);
    EXPECT_THROW(parse_date("1970/01/32"), std::out_of_range);

    EXPECT_THROW(parse_date(""), std::invalid_argument);
}

TEST(Parsers, RelativeDate) {
    EXPECT_EQ(parse_date("TODAY"), LocalClock::today());
    EXPECT_EQ(parse_date("tOdAy"), LocalClock::today());

    EXPECT_EQ(parse_date("TOMORROW"), LocalClock::today() + 1_d);
    EXPECT_EQ(parse_date("YESTERDAY"), LocalClock::today() - 1_d);

    EXPECT_THROW(parse_date("TOD"), std::invalid_argument);
}

TEST(Parsers, DateOperators) {
    EXPECT_EQ(parse_date("1970/01/01+1d"), Date{1_d});
    EXPECT_EQ(parse_date("1970/01/01+2d"), Date{2_d});
    EXPECT_EQ(parse_date("1970/01/02+1d"), Date{2_d});

    EXPECT_EQ(parse_date("1970/01/02-1d"), Date{0_d});

    EXPECT_EQ(parse_date("1970/01/01+1d-2m+3y-4w"), Date{0_d} + 1_d - 2_M + 3_y - 4_w);

    EXPECT_EQ(parse_date("1970/01/01+123d"), Date{0_d} + 123_d);

    EXPECT_EQ(parse_date("1970/01/01+0d-0m+0y-0w"), Date{0_d});

    EXPECT_THROW(parse_date("1970/01/01/1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970/01+1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970/01/01/01+1d"), std::invalid_argument);

    EXPECT_THROW(parse_date("1970/01/01+1dd"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970/01/01++1d"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970/01/01+d"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970/01/01+1"), std::invalid_argument);
    EXPECT_THROW(parse_date("1970/01/01+1-1d"), std::invalid_argument);

    EXPECT_THROW(parse_date("1970/01/01+1h-1m+1s"), std::invalid_argument);

    EXPECT_THROW(parse_date("1970/01/01abcgarbageetc"), std::invalid_argument);

    EXPECT_NO_THROW(parse_date("1970/01/01+65535d"));
    EXPECT_THROW(parse_date("1970/01/01+65536d"), std::out_of_range);
}

TEST(Parsers, RelativeDateOperators) {
    EXPECT_EQ(parse_date("TODAY+1d"), LocalClock::today() + 1_d);
    EXPECT_EQ(parse_date("TODAY+2d"), LocalClock::today() + 2_d);

    EXPECT_EQ(parse_date("TODAY-1d"), LocalClock::today() - 1_d);

    EXPECT_EQ(parse_date("TODAY+1d-2m+3y-4w"), LocalClock::today() + 1_d - 2_M + 3_y - 4_w);

    EXPECT_EQ(parse_date("TODAY+123d"), LocalClock::today() + 123_d);

    EXPECT_EQ(parse_date("TODAY+0d-0m+0y-0w"), LocalClock::today());

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
    EXPECT_EQ(parse_time("00:00:00"), Timestamp{});
    EXPECT_EQ(parse_time("23:59:59"), Timestamp{86399_s});
    EXPECT_EQ(parse_time("23:59"), Timestamp{86340_s});

    EXPECT_THROW(parse_time("24:00:00"), std::out_of_range);
    EXPECT_THROW(parse_time("00:60:00"), std::out_of_range);
    EXPECT_THROW(parse_time("00:00:60"), std::out_of_range);

    EXPECT_THROW(parse_time(""), std::invalid_argument);
}

TEST(Parsers, TimeOperators) {
    EXPECT_EQ(parse_time("00:00+1h"), Timestamp{3600_s});
    EXPECT_EQ(parse_time("00:00:59+1h"), Timestamp{3659_s});
    EXPECT_EQ(parse_time("00:00+2h"), Timestamp{7200_s});

    EXPECT_EQ(parse_time("00:00+1h-59m+1s"), Timestamp{61_s});
    EXPECT_EQ(parse_time("00:00+0h-0m+0s"), Timestamp{});

    EXPECT_EQ(parse_time("23:59:59+1s"), Timestamp{});

    EXPECT_THROW(parse_time("00:00/1h"), std::invalid_argument);

    EXPECT_THROW(parse_time("00:00+1hh"), std::invalid_argument);
    EXPECT_THROW(parse_time("00:00++1h"), std::invalid_argument);
    EXPECT_THROW(parse_time("00:00+h"), std::invalid_argument);
    EXPECT_THROW(parse_time("00:00+1"), std::invalid_argument);
    EXPECT_THROW(parse_time("00:00+1+1h"), std::invalid_argument);

    EXPECT_THROW(parse_time("00:00+1y-1m+1d"), std::invalid_argument);

    EXPECT_THROW(parse_time("00:00abcgarbageetc"), std::invalid_argument);

    EXPECT_NO_THROW(parse_time("00:00+65535h"));
    EXPECT_THROW(parse_time("00:00+65536h"), std::out_of_range);
}

TEST(Parsers, RelativeTime) {
    EXPECT_EQ(parse_time("NOW"), LocalClock::since_midnight());
    EXPECT_EQ(parse_time("nOw"), LocalClock::since_midnight());

    EXPECT_THROW(parse_time("NO"), std::invalid_argument);
}

TEST(Parsers, RelativeTimeOperators) {
    EXPECT_EQ(parse_time("NOW+1h"), LocalClock::since_midnight() + 1_h);
    EXPECT_EQ(parse_time("NOW+2h"), LocalClock::since_midnight() + 2_h);

    EXPECT_EQ(parse_time("NOW+1h-59m+1s"), LocalClock::since_midnight() + 1_h - 59_m + 1_s);
    EXPECT_EQ(parse_time("NOW+0h-0m+0s"), LocalClock::since_midnight());

    EXPECT_THROW(parse_time("NOW/1h"), std::invalid_argument);

    EXPECT_THROW(parse_time("NOW+1hh"), std::invalid_argument);
    EXPECT_THROW(parse_time("NOW++1h"), std::invalid_argument);
    EXPECT_THROW(parse_time("NOW+h"), std::invalid_argument);
    EXPECT_THROW(parse_time("NOW+1"), std::invalid_argument);
    EXPECT_THROW(parse_time("NOW+1+1h"), std::invalid_argument);
    EXPECT_THROW(parse_time("+1h"), std::invalid_argument);

    EXPECT_THROW(parse_time("NOW+1y-1m+1d"), std::invalid_argument);

    EXPECT_THROW(parse_time("NOWabcgarbageetc"), std::invalid_argument);

    EXPECT_NO_THROW(parse_time("NOW+65535h"));
    EXPECT_THROW(parse_time("NOW+65536h"), std::out_of_range);
}

TEST(Parsers, TimePoint) {
    EXPECT_EQ(parse_timepoint("1970/01/01"), TimePoint{86399_s});
    EXPECT_EQ(parse_timepoint("1970/01/01_23:59:59"), TimePoint{86399_s});
    EXPECT_EQ(parse_timepoint("1970/01/01_01:00"), TimePoint{3600_s});
    EXPECT_EQ(parse_timepoint("1970/01/01+1d_00:00+2h"), TimePoint{93600_s});

    EXPECT_EQ(parse_timepoint("TODAY_NOW"),
              (DateTime{LocalClock::today(), LocalClock::since_midnight()}.time_point()));

    EXPECT_EQ(
        parse_timepoint("TODAY+1d_NOW+2h"),
        (DateTime{LocalClock::today() + 1_d, LocalClock::since_midnight() + 2_h}.time_point()));

    EXPECT_THROW(parse_timepoint("1970/01/01 23:59:59"), std::invalid_argument);

    EXPECT_THROW(parse_timepoint("garbage"), std::invalid_argument);
    EXPECT_THROW(parse_timepoint(""), std::invalid_argument);
}

TEST(Parsers, RepeatRule) {
    Lines::TaskRepeatRule res = parse_repeat_rule("3d");

    EXPECT_EQ(std::get<Lines::TaskRepeat::EveryUnit>(res.repeat_type).unit_str, "d");
    EXPECT_EQ(std::get<Lines::TaskRepeat::EveryUnit>(res.repeat_type).interval, 3_d);

    Lines::TaskRepeatRule res2 = parse_repeat_rule("mon");

    auto vectors_eq = [](const auto &lhs, const auto &rhs) -> bool {
        if (lhs.size() != rhs.size()) {
            return false;
        }
        for (std::size_t i{}; i < lhs.size(); ++i) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }
        return true;
    };

    EXPECT_TRUE(
        vectors_eq(std::get<Lines::TaskRepeat::EveryWeekday>(res2.repeat_type).weekdays,
                   std::vector<Lines::Temporal::Weekday>{Lines::Temporal::Weekday::Monday}));

    Lines::TaskRepeatRule res3 = parse_repeat_rule("mon,wed");

    EXPECT_TRUE(
        vectors_eq(std::get<Lines::TaskRepeat::EveryWeekday>(res3.repeat_type).weekdays,
                   std::vector<Lines::Temporal::Weekday>{Lines::Temporal::Weekday::Monday,
                                                         Lines::Temporal::Weekday::Wednesday}));

    Lines::TaskRepeatRule res4 = parse_repeat_rule("mon,wed.sat");

    EXPECT_TRUE(
        vectors_eq(std::get<Lines::TaskRepeat::EveryWeekday>(res4.repeat_type).weekdays,
                   std::vector<Lines::Temporal::Weekday>{
                       Lines::Temporal::Weekday::Monday, Lines::Temporal::Weekday::Wednesday,
                       Lines::Temporal::Weekday::Thursday, Lines::Temporal::Weekday::Friday,
                       Lines::Temporal::Weekday::Saturday}));
}
