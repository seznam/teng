/*
 * Teng -- a general purpose templating engine.
 * Copyright (C) 2004  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Naskove 1, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:teng@firma.seznam.cz
 *
 *
 * $Id: tengfunction.cc,v 1.18 2008-11-20 23:32:29 burlog Exp $
 *
 * DESCRIPTION
 * Teng processor function (like len, substr, round or date)
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Stepan Skrob <stepan@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-26  (honza)
 *             Created.
 * 2004-05-08  (vasek)
 *             Polished code.
 * 2005-06-21  (roman)
 *             Win32 support.
 * 2018-06-14  (burlog)
 *             Polished.
 */

#ifndef TENGFUNCTIONDATE_H
#define TENGFUNCTIONDATE_H

#include <string>
#include <cstring>
#include <ctime>
#include <sys/time.h>

#include "platform.h"
#include "functionutil.h"
#include "function.h"

namespace Teng {
namespace builtin {
namespace {

/** Count unixtime from struct tm - respect timezone
 * @param dateTime source time structure
 * @param counted unixtime
 * @return -1 if error occurred, 0 OK
 */
int unixtime(struct tm dateTime, time_t &res) {
    // set tzname, timezone and other system varibales
    tzset();
    // let maketime set dst flag
    dateTime.tm_isdst = -1;
    // maketime untouch this if some error occurred
    dateTime.tm_wday = 7;
    // save datetime timezone offset
    long gmtoff = dateTime.tm_gmtoff;

    // make time :)
    res = mktime(&dateTime);
    // substract localtime timezone (respect dst) - maketime should use
    // timezone from datetime but don't do this; we must recover it
#ifdef HAVE_ALTZONE
    res -= (dateTime.tm_isdst > 0)? ::altzone: ::timezone;
#else
    res -= (dateTime.tm_isdst > 0)? ::timezone - 3600: ::timezone;
#endif
    // substract datetime gmt offset
    res -= gmtoff;

    // is there any error?
    if ((res <= -1) || (dateTime.tm_wday == 7)) {
        return -1;
    }
    return 0;
}

/** makes struct tm from string
 * @param str source
 * @param dateTime destination
 * @return 0 OK, -1 error
 * */
int parseDateTime(Ctx_t &ctx, const string_view_t &str, struct tm &dateTime) {
    char buf4[5];
    char buf2[3];
    char *end; // is set by strtoul
    const char *pos = str.data();
    memset(&dateTime, 0, sizeof(dateTime));

    auto warn = [&] (const std::string &msg) {
        logWarning(
            ctx.err,
            ctx.pos,
            "parseDateTime(): " + msg
            + "; use YYYY-MM-DD[ HH:MM:SS[+ZHZM]] format"
        );
    };

    buf4[4] = buf2[2] = 0;

    strncpy(buf4, pos, 4);
    dateTime.tm_year = strtoul(buf4, &end, 10)-1900;
    if (end != (buf4 + 4)) {
        warn("invalid format of year");
        return -1;
    }
    pos += 4;
    if (*pos == '-') ++pos;

    strncpy(buf2, pos, 2);
    dateTime.tm_mon = strtoul(buf2, &end, 10) - 1;
    if (end != (buf2 + 2)) {
        warn("invalid format of month");
        return -1;
    }
    pos += 2;
    if (*pos == '-') ++pos;

    strncpy(buf2, pos, 2);
    dateTime.tm_mday = strtoul(buf2, &end, 10);
    if (end != (buf2 + 2)) {
        warn("invalid format of day");
        return -1;
    }
    pos += 2;

    switch(*pos++) {
    case 0:
        goto endFunction;
    case ' ':
    case 'T':
        break;
    default:
        warn("expected 'T' or ' ' as date/time separator");
        return -1;
    }

    strncpy(buf2, pos, 2);
    dateTime.tm_hour = strtoul(buf2, &end, 10);
    if (end != (buf2 + 2)) {
        warn("invalid format of hour");
        return -1;
    }
    pos += 2;
    if (*pos++ != ':') {
        warn("expected ':' as hour/minute separator");
        return -1;
    }

    strncpy(buf2, pos, 2);
    dateTime.tm_min = strtoul(buf2, &end, 10);
    if (end != (buf2 + 2)) {
        warn("invalid format of minute");
        return -1;
    }
    pos += 2;
    if (*pos++ != ':') {
        warn("expected ':' as minute/second separator");
        return -1;
    }

    strncpy(buf2, pos, 2);
    dateTime.tm_sec = strtoul(buf2, &end, 10);
    if (end != (buf2 + 2)) {
        warn("invalid format of second");
        return -1;
    }
    pos += 2;

    if (*pos) {
        if ((*pos != '-') && (*pos != '+')) {
            warn("expected '+' or '-'");
            return -1;
        }
        char sign = *pos;
        pos += 1;

        // hours
        strncpy(buf2, pos, 2);
        dateTime.tm_gmtoff = strtoul(buf2, &end, 10);
        if ((end != (buf2 + 2)) || (dateTime.tm_gmtoff < 0)
                                || (dateTime.tm_gmtoff > 12)) {
            warn("expected ZH");
            return -1;
        }
        pos += 2;

        // omit ':' if present
        if (*pos == ':') pos += 1;

        // minutes
        strncpy(buf2, pos, 2);
        int minutes = strtoul(buf2, &end, 10) * 60 * 60;
        if ((end != (buf2 + 2)) || (minutes < 0) || (minutes > 60)) {
            warn("expected ZM");
            return -1;
        }
        dateTime.tm_gmtoff = ((dateTime.tm_gmtoff * 60 * 60) + minutes * 60)
                             * (sign == '-'? -1: +1);
    }

endFunction:
    if (dateTime.tm_year<0) {
        dateTime.tm_wday=-1;
        dateTime.tm_yday=-1;

    } else {
        struct tm tmpt = dateTime;
        mktime(&tmpt);
        dateTime.tm_wday = tmpt.tm_wday;
        dateTime.tm_yday = tmpt.tm_yday;
    }
    return 0;
}


/** Parse date-setup string and add specifiead word into string.
 * Input string contains month and day names and is formated as follows:
 * |January|February|...|December|Jan|Feb|...|Dec\
 * |Sunday|Monday|...|Saturday|Sun|Mon|...|Sat|
 * @return Status: 0=ok, -1=error.
 * @param index Which substring put from setup-string into output.
 * @param setup The date-setup string.
 * @param out output string. */
int
addDateString(uint32_t index, const string_view_t &setup, std::string &out) {
    // find the proper word
    auto isetup = setup.begin();
    for (++index; index > 0; --index) {
        // find next '|"
        for (; (isetup != setup.end()) && (*isetup != '|');
             ++isetup);
        // runaway => error
        if (isetup == setup.end()) return -1;

        // move after pipe
        ++isetup;
    }

    // not found => error (must be zero)
    if (index) return -1;

    auto esetup = isetup;
    // find terminating delimiter
    while  ((esetup != setup.end()) && (*esetup != '|'))
        esetup++;

    // check for valid string
    if ((esetup == setup.end()) || (*esetup != '|'))
        return -1;

    // append string
    out.append(isetup, esetup);

    return 0;
}

template <typename T1, typename... T>
void formatValue(std::string &out, const char *format, T1 v1, T &&...v) {
    char buf[60];
    auto len = snprintf(buf, sizeof(buf), format, v1, std::forward<T>(v)...);
    out.append(buf, (std::size_t(len) > sizeof(buf))? sizeof(buf): len);
}

/** Function for formating dates like strftime() from struct tm.
 * @return Status: 0=ok, -1=error.
 * @param format Format string (like C strftime).
 * @param setup String with literals of month/day names.
 * @param dateTime Date/time for formating.
 * @param output Result (formated string). */
int formatBrokenDate(
    const string_view_t &format,
    const string_view_t &setup,
    const struct tm &dateTime,
    std::string &output
) {
    // for all chars in format string
    for (auto ptr = format.begin(); ptr != format.end(); ++ptr) {
        // if formating char '%'  found
        if (*ptr == '%') {
            ++ptr; //next char
            if (ptr == format.end()) {
                output.push_back('%');
                break;
            }

            // switch on next char
            switch (*ptr) {
            case 'Y': //year (1970)
                formatValue(output, "%d", dateTime.tm_year + 1900);
                break;

            case 'y': //last two digits of the year (00..99)
                formatValue(output, "%02d", (dateTime.tm_year + 1900) % 100);
                break;

            case 'u': //day of week 1=monday (1..7)
                formatValue(output, "%d", (dateTime.tm_wday + 6) % 7 + 1);
                break;

            case 'w': //day of week 0=sunday (0..6)
                formatValue(output, "%d", dateTime.tm_wday);
                break;

            case 'm': //month (01..12)
                formatValue(output, "%02d", dateTime.tm_mon + 1);
                break;

            case 'n': //month (1..12)
                formatValue(output, "%d", dateTime.tm_mon + 1);
                break;

            case 'd': //day (01..31)
                formatValue(output, "%02d", dateTime.tm_mday);
                break;

            case 'e': //day (1..31)
                formatValue(output, "%d", dateTime.tm_mday);
                break;

            case 'H': //hour (00..23)
                formatValue(output, "%02d", dateTime.tm_hour);
                break;

            case 'k': //hour (0..23)
                formatValue(output, "%d", dateTime.tm_hour);
                break;

            case 'j': //day of the year (000..366)
                formatValue(output, "%03d", dateTime.tm_yday);
                break;

            case 'I': //hour (01..12)
                formatValue(output, "%02d", (dateTime.tm_hour % 12) + 1);
                break;

            case 'r': //time 12hr AM/PM "%I:%M:%S %p"
                formatValue(output, "%02d:%02d:%02d %s",
                            (dateTime.tm_hour % 12) + 1,
                            dateTime.tm_min, dateTime.tm_sec,
                            dateTime.tm_hour <= 11 ? "AM" : "PM");
                break;

            case 'T': //time 24hr "%H:%M:%S"
                formatValue(output, "%02d:%02d:%02d",
                            dateTime.tm_hour, dateTime.tm_min,
                            dateTime.tm_sec);
                break;

            case 'R': //time 24hr "%H:%M"
                formatValue(output, "%02d:%02d",
                            dateTime.tm_hour, dateTime.tm_min);
                break;

            case 'l': //hour (1..12)
                formatValue(output, "%d", (dateTime.tm_hour % 12) + 1);
                break;

            case 'M': //minute (00..59)
                formatValue(output, "%02d", dateTime.tm_min);
                break;

            case 'P': //am/pm
                output.append(dateTime.tm_hour <= 11 ? "am" : "pm");
                break;

            case 'p': //AM/PM
                output.append(dateTime.tm_hour <= 11 ? "AM" : "PM");
                break;

            case 's': // unix time
                {
                    time_t ut;
                    if (!unixtime(dateTime, ut)) formatValue(output, "%ld", ut);
                }
                break;

            case 'S': //second (00..59), but possibly up to 61
                formatValue(output, "%02d", dateTime.tm_sec);
                break;

            case 'B': //full month name (January..December)
                if (dateTime.tm_mon >= 0 && dateTime.tm_mon <= 11) {
                    addDateString(dateTime.tm_mon, setup, output);
                }
                break;

            case 'h':
            case 'b': //abbreviated month name (Jan..Dec)
                if (dateTime.tm_mon >= 0 && dateTime.tm_mon <= 11) {
                    addDateString(dateTime.tm_mon + 12, setup, output);
                }
                break;

            case 'A': //full weekday name (Sunday..Saturday)
                if (dateTime.tm_wday >= 0 && dateTime.tm_wday <= 6) {
                    addDateString(dateTime.tm_wday + 24, setup, output);
                }
                break;

            case 'a': //abbreviated day name (Sun..Sat)
                if (dateTime.tm_wday >= 0 && dateTime.tm_wday <= 6) {
                    addDateString(dateTime.tm_wday + 31, setup, output);
                }
                break;

            case 'z':
                {
                    char buffer[32];
                    strftime(buffer, sizeof(buffer), "%z", &dateTime);
                    output.append(buffer);
                }
                break;

            case 'Z':
                {
                    struct tm tmpt = dateTime;
                    tmpt.tm_isdst = -1;
                    mktime(&tmpt);
                    output.append(tmpt.tm_zone);
                }
                break;

            case '%': //percent sign '%'
            default: //and other characters
                output.push_back(*ptr);
            }
        } else {
            //other chars
            output.push_back(*ptr);
        }
    }

    // success
    return 0;
}

/** strftime for string input
 * @param format format string (like C strftime)
 * @param setup string for month/day names
 * @param date date to format
 * @param output result
 * @return 0 OK, -1 error
 * */
int formatStringDate(
    Ctx_t &ctx,
    const string_view_t &format,
    const string_view_t &setup,
    const string_view_t &date,
    std::string &output
) {
    struct tm dateTime;
    if (parseDateTime(ctx, date, dateTime)) return -1;
    return formatBrokenDate(format, setup, dateTime, output);
}

/** Like strftime for time_t since Epoch input
 *
 * @param format format string (like C strftime)
 * @param setup string for month/day names
 * @param date date to format
 * @param output result
 *
 * @return 0 OK, -1 error
 */
int formatTime_tDate(
    const string_view_t &format,
    const string_view_t &setup,
    time_t date,
    std::string &output
) {
    struct tm dateTime;
    if (!localtime_r(&date, &dateTime)) return -1;
    return formatBrokenDate(format, setup, dateTime, output);
}

/** Like strftime for time_t since Epoch input
 *
 * @return 0 OK, -1 error
 */
int to_number(const string_view_t &text, time_t &res) {
    // int value
    char *err = nullptr;
    auto int_value = strtol(text.data(), &err, 10);
    if (*err == '\0') return res = int_value, 0;

    // real value
    auto real_value = strtod(text.data(), &err);
    if (*err == '\0') return res = real_value, 0;

    // not a number
    return -1;
}

} // namespace

/** Second count since Epoch. (with precision of microsecond)
 *
 * @param args Teng function arguments
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t now(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 0)
        return wrongNumberOfArgs(ctx, "now", 0);

    // ensure that we are in runtime environment
    ctx.runtime_ctx_needed();

    // there is no need care about timezone
    // localtime and maketime work with actual timezone which is now :)
    struct timeval tv;
    gettimeofday(&tv, /*&tz*/nullptr);
    return Result_t(tv.tv_sec + tv.tv_usec / 1000000.0);
}

/** Converts string to timestamp.
 *
 * @param args Teng function arguments
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t timestamp(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "timestamp", 1);
    return args[0].print([&] (const string_view_t &arg) {
        struct tm dateTime;
        if (parseDateTime(ctx, arg, dateTime))
            return failed(ctx, "timestamp", "Can't parse date");

        time_t res = 0;
        if (unixtime(dateTime, res) == -1) {
            static auto *msg = "Can't convert parsed date to timestamp";
            return failed(ctx, msg, "timestamp");
        }

        return Result_t(res);
    });
}

/** strftime like Teng function. Uses user month / day names
 *
 * @param args Teng function arguments
 * @param ctx Teng function ctx
 * @param result Teng function result
 */
Result_t date(Ctx_t &ctx, const Args_t &args) {
    if ((args.size() < 2) || (args.size() > 3))
        return wrongNumberOfArgs(ctx, "date", 2, 3);
    auto iarg = args.rbegin();

    // 0: format
    if (!iarg->is_string_like())
        return failed(ctx, "date", "First arg must be string");
    auto &format = *(iarg++);

    // 1: date
    auto &value = *(iarg++);

    // 2: setup [optional]
    string_view_t setup;
    if (iarg != args.rend()) {
        if (!iarg->is_string_like())
            return failed(ctx, "date", "Thrid arg must be string");
        setup = iarg->string();
    }

    // string formatting
    time_t time_value = 0;
    if (value.is_string_like()) {
        // if string contains a number skip string formatting
        if (!to_number(value.string(), time_value)) {
            std::string res;
            auto format_str = format.string();
            auto value_str = value.string();
            if (formatStringDate(ctx, format_str, setup, value_str, res))
                return failed(ctx, "date", "Formatting failed");
            return Result_t(std::move(res));
        }
    } else time_value = value.integral();

    // number formatting
    std::string res;
    if (formatTime_tDate(format.string(), setup, time_value, res))
        return failed(ctx, "date", "Formatting failed");
    return Result_t(res);
}

/** Converts second to time format hours:minute:second.
 *
 * @param args Function arguments (list of values).
 * @param ctx Teng function ctx.
 * @param result Function's result value.
 */
Result_t sectotime(Ctx_t &ctx, const Args_t &args) {
    if (args.size() != 1)
        return wrongNumberOfArgs(ctx, "sectotime", 1);

    // check args
    if (!args.back().is_number())
        return failed(ctx, "sectotime", "Arg must be a number");

    // split
    auto sec = args.back().integral();
    auto hours = sec / 3600;
    auto minutes = (sec % 3600) / 60;
    auto seconds = sec % 60;

    // convert to string
    char buf[64];
    snprintf(buf, sizeof(buf), "%ld:%02ld:%02ld", hours, minutes, seconds);
    return Result_t(buf);
}

} // namespace builtin
} // namespace Teng

#endif /* TENGFUNCTIONDATE_H */

