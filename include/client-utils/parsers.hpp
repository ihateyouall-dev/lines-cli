#pragma once

#include <string>
#include <lines/temporal/datetime.hpp>
#include <lines/temporal/timepoint.hpp>

void range_error(std::string_view prefix, std::string_view range_str);

auto parse_date(const std::string &str) -> Lines::Temporal::Date;

auto parse_time(const std::string &str) -> Lines::Temporal::Timestamp;

auto parse_timepoint(const std::string &str) -> Lines::Temporal::TimePoint;
