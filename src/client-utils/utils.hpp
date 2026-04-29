#pragma once

#include "lines/tasks/task.hpp"
#include "lines/temporal/timepoint.hpp"

#include <string>

auto confirm() -> bool;

auto date_str(const Lines::Temporal::Date &date) -> std::string;

auto timepoint_str(const Lines::Temporal::TimePoint &tp) -> std::string;
auto timepoint_str_s(const Lines::Temporal::TimePoint &tp) -> std::string;

auto tags_str(const Lines::Task &task) -> std::string;

auto completion_sign(const Lines::Task &task) -> std::string;

auto task_str_unfolded(const Lines::Task &task) -> std::string;

auto task_str(const Lines::Task &task) -> std::string;

auto today() -> Lines::Temporal::Date;
auto today_str() -> std::string;

auto tomorrow() -> Lines::Temporal::Date;
auto tomorrow_str() -> std::string;
