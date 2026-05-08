#pragma once

#include <string>

namespace Lines::Manuals {
static const std::string repeat_manual = R"(Task repeat rules.

The difference between regular task and task with a repeat rule is that
repeatable tasks do not have a "completed" or "uncompleted" state.
Every time you complete a repeatable task, it only moves task's deadline
to the next occurrence, which is calculated using the repeat rule.

There are two types of repeat rules:
	1. Every N units.   (Like "every 5 days", "every week", "every 7 hours", etc.)
	2. Every <weekday>. (Like "every monday", "every saturday and sunday",
                                                  "every tuesday to friday", etc.)
Syntax:
	1. N(s|m|h|d|w|mo|y).
           Examples: "5d", "1w", "7h", "3mo"

	2. wd1,wd2  ->  multiple weekdays (WD1 AND WD2)
           Example: "mon,wed"
           wd1.wd2  ->  weekday range (FROM WD1 TO WD2)
           Example: "mon.wed"

           Weekday names:
           mon tue wed thu fri sat sun

           Both can be combined in one rule, like "mon,wed.sat")";

static const std::string timepoints_manual = R"(Time points.

Time point is a specific moment in time.
In lines-cli it is represented as DATE_TIME or DATE (if time is omitted, it defaults to 23:59:59).


Supported DATE format is YYYY/MM/DD
Supported TIME format is HH:MM[:SS] (HH:MM is simillar to HH:MM:00).

Also, there is shortcuts for dates and time, like "TODAY", "TOMORROW" and "YESTERDAY" for dates,
and "NOW" for time.
If the meaning of these shortcuts is not obvious, here is an example:

Let's say that today is 2026/05/08, and time on clock is exactly 12 o'clock, then:
	TODAY     is 2026/05/08
	TOMORROW  is 2026/05/09
	YESTERDAY is 2026/05/07
	NOW       is 12:00:00
(WARNING: Shortcuts use local time of your machine, you can adjust
behavior of shortcuts at your config (WIP))

You can also define date and time relatively, for this
purpose there is a thing called "temporal operators", it lets
you do date and time arithmetic to define time point.

Format of temporal operators:
        [+-]N(d|w|m|y) for dates
        [+-]N(s|m|h)   for time

Examples:
	"2026/01/01"                (2026/01/01 23:59:59)
	"2026/01/01_12:34:56"
	"2026/01/01_12:34"          (2026/01/01 12:34:00)
        "2026/01/01+1d+1m-1y"       (2025/02/02 23:59:59)
        "2026/01/01_12:00+1s+1m-1h" (2026/01/01 11:01:01)
        "TODAY"                     (today's date 23:59:59)
        "TODAY_NOW"                 (today's date and current time)


To reduce cognitive load, lines-cli also highlights time points
regarding to how much time left to them. Time points that were passed is highlighted in red color,
if only 24 or less hours left until time point, it's highlight in yellow color, other time points
highlight in green color)";
} // namespace Lines::Manuals
