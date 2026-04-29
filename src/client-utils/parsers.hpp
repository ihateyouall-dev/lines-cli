#pragma once

#include "lines/temporal/date.hpp"
#include "lines/temporal/timepoint.hpp"
#include "lines/temporal/timestamp.hpp"

#include <string>

void throw_range_error(std::string_view prefix, std::string_view range_str);

auto parse_date(const std::string &str) -> Lines::Temporal::Date;

auto parse_time(const std::string &str) -> Lines::Temporal::Timestamp;

auto parse_timepoint(const std::string &str) -> Lines::Temporal::TimePoint;
