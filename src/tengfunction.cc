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
 * $Id: tengfunction.cc,v 1.3 2004-09-09 15:32:16 solamyl Exp $
 *
 * DESCRIPTION
 * Teng processor funcction (like len, substr, round or date)
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Stepan Skrob <stepan@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-26  (honza)
 *             Created.
 * 2004-05-08  (vasek)
 *             Polished code.
 *
 * TODO
 * 2004-05-08  (vasek)
 *             * Fix all functions:
 *               - don't use indices but iterators
 *               - don't use goto's (grrr)
 *               - don't use signed where unsigned is appropriate
 *                 (get rid of terrible C-style casts)
 *               - heavy use of error logging is appreciated!
 *               - in short: make it C++ :-)
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>

#include "tengfunction.h"

using namespace std;

using namespace Teng;

/** strlen for UTF-8 string.
 * @param str string
 * @return length of str
 * */
static int strlenUTF8(const string &_str) {
    string str = _str;
    int chars = 0;
    string::iterator end = str.end();
    
    for (string::iterator istr = str.begin(); istr != end; ) {
        int bytes = 0;
        char tmp = *istr;
        
        while (tmp & 0x80) {
            ++bytes;
            tmp <<= 1;
        }
        
        if (!bytes) {
            istr++;
            chars++;
            continue;
        }
        if ((bytes == 1) || (bytes > 6)) {
            istr++;
            chars++;
            continue;
        } 
        --bytes;
        string::iterator bstr = istr + 1;
        for (; (bstr != end) && bytes; ++bstr, --bytes) {
            if ((*bstr & 0xC0) != 0x80) break;
        }
        if (bytes) {
            while (istr != bstr) {
                istr++;
                chars++;
            }
            continue;
        }
        else
            {
                istr = bstr;
                chars++;
            }
    }
    return chars;
}

/** Python-like substr for UTF-8 string.
 * @param str source string
 * @param result destination string
 * @param s start index
 * @param e end index
 * */
static void substrUTF8(const string &str, string &result, int s, int e,
                       string &p1, string &p2)
{
    int l = strlenUTF8(str);
    if (s < 0) s = l + s;
    if (e < 0) e = l + e;
    if (s <= 0) {
        s = 0;
        p1 = "";
    }
    if (e >= l) {
        e = l;
        p2 = "";
    }
    if (!l || s >= l || e <= 0 || e <= s) {
        result = p1 + p2;
        return;
    }
    else result = "";
    string::const_iterator end = str.end();
    int i=0;
    for (string::const_iterator istr = str.begin(); istr != end; ) {
        int bytes = 0;
        char tmp = *istr;
        
        while (tmp & 0x80) {
            ++bytes;
            tmp <<= 1;
        }
        
        if (!bytes) {
            if (i >= s) result += *istr;
            istr++;
            i++;
            if (i >= e) goto beginEnd;
           continue;
        }
        if ((bytes == 1) || (bytes > 6)) {
            if (i >= s) result += *istr;
            istr++;
            i++;
            if (i >= e) goto beginEnd; 
            continue;
        } 
        --bytes;
        string::const_iterator bstr = istr + 1;
        for (; (bstr != end) && bytes; ++bstr, --bytes) {
            if ((*bstr & 0xC0) != 0x80) break;
        }
        if (bytes)
            while (istr != bstr) {
                if (i >= s) result += *istr;
                istr++;
                i++;
                if (i >= e) goto beginEnd;
            }
        else{
            while (istr != bstr) {
                if (i >= s) result += *istr;
                istr++;
            }
            i++;
            if (i >= e) goto beginEnd;
        }
    }
 beginEnd:
    result = p1 + result + p2;
}

/** Python-like substr
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionSubstr(const vector<ParserValue_t> &args,
                              const Processor_t::FunctionParam_t &setting,
                              ParserValue_t &result)
{
    int s, e;
    result.setString("undefined");
    if (args.size() < 3 || args.size() > 5) return -1;
    string p1, p2; // default empty string to begin and end
    s = 0;

    switch(args.size()) {
    case 5:
        p2 = args[s++].stringValue;
        p1 = args[s++].stringValue;
        break;

    case 4:
        p2 = args[s++].stringValue;
        p1 = p2;
        break;

    default:
        break;
    }

    ParserValue_t c(args[s++]);
    ParserValue_t b(args[s++]);
    ParserValue_t a(args[s++]);
    
    b.validateThis();
    c.validateThis();
    if ((b.type != ParserValue_t::TYPE_INT) ||
        (c.type != ParserValue_t::TYPE_INT))
        return -2;
    s = b.integerValue;
    e = c.integerValue;
    if (setting.encoding == "utf-8") {
        substrUTF8(a.stringValue, result.stringValue, s, e, p1, p2);
        /* result.validateThis(); */
        return 0;
    }

     if (s < 0) s = a.stringValue.size() + s;
     if (e < 0) e = a.stringValue.size() + e;
     if (s <= 0) {
         s = 0;
         p1 = "";
     }
     if (e >= (int)a.stringValue.size()) {
         e = a.stringValue.size();
         p2 = "";
     }
     
     if (s >= ((int)a.stringValue.size()) || e <= 0 || e <= s)
         result.setString(p1 + p2);
     else {
         e -= s;
         result.setString(p1 + a.stringValue.substr(s, e) + p2);
     }
     return 0;
}

 /** Find real indexes into string for UTF-8 substr.
  * @param str source string
  * @param s start index
  * @param e end index
  * */
static void substrIndexUTF8(const string &str, int &s, int &e) {
    int l = strlenUTF8(str), index, end = str.size(),
        sset = 0, chars = 0;
    if (!l) {
    empty:
        s = 0;
        e = 1;
        return;
    }

    if (s < 0) s = l + s;
    if (e < 0) e = l + e;
    if (s < 0) s = 0;
    if (e > l) e = l;
    if (s >= l || e <= 0 || e <= s) goto empty;
    if (!s) sset = 0;
    for (index = 0; index < end; ) {
        int bytes = 0;
        char tmp = str[index];
        // compute number of bytes in char
        while (tmp & 0x80) {
            ++bytes;
            tmp <<= 1;
        }
        // ASCII char
        if (!bytes) {
            chars++;
            index++;
            if (!sset && chars == s) {
                s = index;
                sset = 1;
            }
            if (chars == e) {
                e = index;
                return;
            }
            continue;
        }
        // wrong char
        if ((bytes == 1) || (bytes > 6)) {
            chars++;
            index++;
            if (!sset && chars == s) {
                s = index;
                sset = 1;
            }
            if (chars == e) {
                e = index;
                return;
            }
            continue;
        } 
        --bytes;
        int bstr = index + 1;
        for (; (bstr < end) && bytes; ++bstr, --bytes) {
            if ((str[bstr] & 0xC0) != 0x80) break;
        }
        while (index < bstr) {
            index++;
        }
        if (!bytes) {
            //    index++;
            chars++;
            if (!sset && chars == s) {
                s = index;
                sset = 1;
            }
            if (chars == e) {
                e = index;
                return;
            }
        }
    }
}

/** Python-like "word safe" substr
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionWordSubstr(const vector<ParserValue_t> &args,
                                  const Processor_t::FunctionParam_t &setting,
                                  ParserValue_t &result)
{
    int s, e;
    result.setString("undefined");
    if ((args.size() < 3) || (args.size() > 5)) return -1;
    s = 0;
    string p1, p2;
    switch (args.size()) {
    case 5:
        p2 = args[s++].stringValue;
        p1 = args[s++].stringValue; break;
    case 4: p2 = args[s++].stringValue;
        p1 = p2;
        break;
    default:
        break;
    }
    
    ParserValue_t c(args[s++]);
    ParserValue_t b(args[s++]);
    ParserValue_t a(args[s++]);
    b.validateThis();
    c.validateThis();
    if (b.type != ParserValue_t::TYPE_INT ||
        c.type != ParserValue_t::TYPE_INT)
        return -2;
    s = b.integerValue;
    e = c.integerValue;
    if (setting.encoding == "utf-8") {
        substrIndexUTF8(a.stringValue, s, e);
        goto wordsplit;
    }
    if (s < 0) s = a.stringValue.size() + s;
    if (e < 0) e = a.stringValue.size() + e;
    if (s < 0) s = 0;
    if (e > (int)a.stringValue.size()) e = a.stringValue.size();
    if (s >= ((int)a.stringValue.size()) || e <= 0 || e <= s)
        result.setString("");
    else {
    wordsplit:
        e -= s;
        if (!isspace(a.stringValue[s + e - 1]))
            while (s + e < (int)a.stringValue.length() &&
                   !isspace(a.stringValue[s + e]))
                e++;
        if (!isspace(a.stringValue[s]))
            while (s > 0 && !isspace(a.stringValue[s - 1]))
                {s--; e++;}
    }

    while (e > 0 && isspace(a.stringValue[s + e - 1])) e--;
    while (e > 0 && isspace(a.stringValue[s])) {e--; s++;}
    int startWhite = 1;
    int endWhite = 1;
    for (int i = 0; i < s && i < (int)a.stringValue.size(); i++)
        if (!isspace(a.stringValue[i])) {
            startWhite = 0;
            break;
        }

    for (int i = s + e; i < (int)a.stringValue.size(); i++)
        if (!isspace(a.stringValue[i])) {
            endWhite = 0;
            break;
        }
    
    if (!startWhite)
        if (!endWhite)
            result.setString(p1 + a.stringValue.substr(s, e) + p2);
        else
            result.setString(p1 + a.stringValue.substr(s, e));
    else
        if (!endWhite)
            result.setString(a.stringValue.substr(s, e) + p2);
        else
            result.setString(a.stringValue.substr(s, e));
    return 0;
}

/** Reorder - pastes strings into result, first arg is format
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionReorder(const vector<ParserValue_t> &args,
                               const Processor_t::FunctionParam_t &setting,
                               ParserValue_t &result)
{
    result.setString("undefined");
    if (args.size() < 1) return -1;
    int ret = 0;
    
    // format string
    const string &format = args[args.size() - 1].stringValue;
    
    // result (formated string) -- reserve some space
    string res;
    res.reserve(2 * format.length());

    // status of automaton
    enum {
        STATUS_DEFAULT, /** < default mode, normal string */
        STATUS_FORMAT,  /** < format mode, we are after % character */
        STATUS_NUMBER,  /** < number mode, we are parsing number (after %{) */
    } status = STATUS_DEFAULT;

    // indicates end-of-string
    const int EOS = ~0;
    
    // parsed index in arguments
    unsigned int index = 0;
    
    // indicates that we want to replace %index by argument
    bool replace = false;

    // position of last %
    string::const_iterator mark = format.begin();
    for (string::const_iterator iformat = format.begin();
         /* forever */; ++iformat) {

        // get next character of EOS when at the string end
        int c = ((iformat == format.end()) ? EOS : *iformat);
        switch (c) {
        case '%':
            switch (status) {
            case STATUS_FORMAT:
                res.push_back('%');
                status = STATUS_DEFAULT;
                break;

            case STATUS_DEFAULT:
                status = STATUS_FORMAT;
                mark = iformat;
                index = 0;
                break;

            case STATUS_NUMBER:
                setting.logger.
                    logError(Error_t::LL_ERROR,
                             "reorder(): '%' not allowed inside '%{}'.");
                ret = -2;
                status = STATUS_DEFAULT;
                res.append(mark, iformat + 1);
                break;
            }
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            // digit
            switch (status) {
            case STATUS_FORMAT:
                // we have single digit after % => get index and reset
                // automaton
                index = (c - '0');
                replace = true;
                status = STATUS_DEFAULT;
                break;

            case STATUS_DEFAULT:
                // normal operation => just pass digit
                res.push_back(c);
                break;

            case STATUS_NUMBER:
                // we are in number => get next digit
                index *= 10;
                index += (c - '0');
                break;
            }
            break;
            
        case '{':
            switch (status) {
            case STATUS_FORMAT:
                // { after % => prepare for number parsing
                status = STATUS_NUMBER;
                index = 0;
                break;

            case STATUS_DEFAULT:
                // OK
                res.push_back(c);
                break;

            case STATUS_NUMBER:
                // ? inside number => error
                setting.logger.
                    logError(Error_t::LL_ERROR,
                             "reorder(): '{' not allowed inside '%{}'.");
                ret = -2;
                status = STATUS_DEFAULT;
                res.append(mark, iformat + 1);
                break;
            }
            break;

        case '}':
            switch (status) {
            case STATUS_FORMAT:
                // } after % => error
                setting.logger.
                    logError(Error_t::LL_ERROR,
                             "reorder(): '}' not allowed after '%'.");
                ret = -2;
                status = STATUS_DEFAULT;
                res.append(mark, iformat + 1);
                break;

            case STATUS_DEFAULT:
                // OK
                res.push_back(c);
                break;

            case STATUS_NUMBER:
                // end of number
                replace = true;
                status = STATUS_DEFAULT;
                break;
            }
            break;

        case EOS:
            // end of string
            switch (status) {
            case STATUS_DEFAULT:
                // OK
                break;

            case STATUS_FORMAT:
            case STATUS_NUMBER:
                // unterminated format
                setting.logger.
                    logError(Error_t::LL_ERROR,
                             "reorder(): runaway argument.");
                ret = -2;
                res.append(mark, iformat);
                break;
            }
            break;

        default:
            // other characters
            switch (status) {
            case STATUS_FORMAT:
            case STATUS_NUMBER:
                // ? after % => error
                {
                    char tc = c;
                    setting.logger.
                        logError(Error_t::LL_ERROR,
                                 ("reorder(): '" + string(&tc, 1)
                                  + "' not allowed inside %{} or after %."));
                }
                ret = -2;
                status = STATUS_DEFAULT;
                res.append(mark, iformat + 1);
                break;

            case STATUS_DEFAULT:
                // OK
                res.push_back(c);
                break;
            }
            break;
        }

        if (replace) {
            if (!index || (index >= args.size())) {
                // invalid index => do not expand and report error
                setting.logger.
                    logError(Error_t::LL_ERROR,
                             ("reorder(): invalid or missing index in format '"
                              + string(mark, iformat + 1) + "'."));
                ret = -2;
                res.append(mark, iformat + 1);
            } else {
                res.append(args[args.size() - 1 - index].stringValue);
            }
            replace = false;
        }

        // on EOS return
        if (c == EOS) break;
    }

    // set result and return error indicator
    result.setString(res);
    return ret;
}

/** Escapes string. Format depends on escaper.
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionEscape(const vector<ParserValue_t> &args,
                              const Processor_t::FunctionParam_t &setting,
                              ParserValue_t &result)
{
    if (args.size() != 1) {
        result.setString("undefined");
        return -1;
    }
    result.setString(setting.escaper.escape(args[0].stringValue));
    return 0;
}

/** Unescapes string. Format depends on escaper.
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionUnescape(const vector<ParserValue_t> &args,
                                const Processor_t::FunctionParam_t &setting,
                                ParserValue_t &result)
{
    if (args.size() != 1) {
        result.setString("undefined");
        return -1;
    }
    result.setString(setting.escaper.unescape(args[0].stringValue));
    return 0;
}

/** Like strlen. Must handle utf-8 if used.
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionLen(const vector<ParserValue_t> &args,
                           const Processor_t::FunctionParam_t &setting,
                           ParserValue_t &result)
{
    result.setString("undefined");
    if (args.size() != 1) return -1;
    if (setting.encoding == "utf-8")
        result.setInteger(strlenUTF8(args[0].stringValue));
    else      
        result.setInteger(args[0].stringValue.length());
    return 0;
}


/** Random integer
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionRandom(const vector<ParserValue_t> &args,
                              const Processor_t::FunctionParam_t &setting,
                              ParserValue_t &result)
{
    result.setString("undefined");
    if (args.size() != 1) return -1;
    ParserValue_t a = args[0].validate();
    if ((a.type != ParserValue_t::TYPE_INT) || (a.integerValue < 1)) {
        setting.logger.
            logError(Error_t::LL_ERROR,
                     "random(): Missing or negative range.");
        return -2;
    }
    // it is not good to use low bits of rand() see man 3 rand for detail
    result.setInteger(static_cast<long>((rand() * (a.integerValue + 0.0))
                                        /(RAND_MAX + 1.0)));
    return 0;
}

 /** String multiplication
  * @param args Teng function arguments
  * @param setting Teng function setting
  * @param result Teng function result
  * @return 0 OK, -1 wrong argument count, -2 other error
  * */
/*static int tengFunctionRepeat(const vector<TengParserValue_t> &args,
                const FunctionParam_t &setting, TengParserValue_t &result)
{
    result.setString("undefined");
    if (args.size() != 2) return -1;
    TengParserValue_t b = args[0].validate();
    if (b.type != TengParserValue_t::TYPE_INT || b.integerValue < 0) return -2;
    TengParserValue_t a = args[1];
   
    result.setString("");
    result.stringValue.reserve(a.stringValue.size() * b.integerValue);
    for (long i = 0; i < b.integerValue; i++) {
         result.stringValue += a.stringValue;
    }
    return 0;
}
*/
 /** Second count since Epoch. (with precission of microsecond)
  * @param args Teng function arguments
  * @param setting Teng function setting
  * @param result Teng function result
  * @return 0 OK, -1 wrong argument count, -2 other error
  * */
static int tengFunctionNow(const vector<ParserValue_t> &args,
                           const Processor_t::FunctionParam_t &setting,
                           ParserValue_t &result)
{
    struct timeval tv;
    //    struct timezone tz;
    
    result.setString("undefined");
    if (args.size()) return -1;
    gettimeofday(&tv, /*&tz*/ NULL);
    result.setReal(tv.tv_sec + tv.tv_usec / 1000000.0);
    return 0;
}

/** makes struct tm from string
 * @param str source
 * @param dateTime destination
 * @return 0 OK, -1 error
 * */
static int parseDateTime(const string &str, struct tm &dateTime) {
    char buf4[5];
    char buf2[3];
    char *end;
    const char *pos = str.c_str();
    memset(&dateTime, 0, sizeof(dateTime));
    
    buf4[4] = buf2[2] = 0;
    
    strncpy(buf4, pos, 4);
    dateTime.tm_year = strtoul(buf4, &end, 10)-1900;
    if (end != (buf4 + 4)) {
        //    invalid format of year
        return -1;
    }
    pos += 4;
    if (*pos == '-') ++pos;
    
    strncpy(buf2, pos, 2);
    dateTime.tm_mon = strtoul(buf2, &end, 10) - 1;
    if (end != (buf2 + 2)) {
        //    invalid format of month
        return -1;
    }
    pos += 2;
    if (*pos == '-') ++pos;
    
    strncpy(buf2, pos, 2);
    dateTime.tm_mday = strtoul(buf2, &end, 10);
    if (end != (buf2 + 2)) {
        //    invalid format of day
        return -1;
    }
    pos += 2;
    
    switch(*pos++) {
    case 0:
        goto endFunction;
    case 'T':
        break;
    default:
        // expected 'T' as date/time separator
        return -1;
    }
    
    strncpy(buf2, pos, 2);
    dateTime.tm_hour = strtoul(buf2, &end, 10);
    if (end != (buf2 + 2)) {
        //  invalid format of hour
        return -1;
    }
    pos += 2;
    if (*pos++ != ':') {
        //   expected ':' as hour/minute separator
        return -1;
    }
    
    strncpy(buf2, pos, 2);
    dateTime.tm_min = strtoul(buf2, &end, 10);
    if (end != (buf2 + 2)) {
        //    invalid format of minute
        return -1;
    }
    pos += 2;
    if (*pos++ != ':') {
        //    expected ':' as minute/second separator");
        return -1;
    }
    
    strncpy(buf2, pos, 2);
    dateTime.tm_sec = strtoul(buf2, &end, 10);
    if (end != (buf2 + 2)) {
        //    invalid format of second");
        return -1;
    }
    pos += 2;
    
    if (*pos) {
        //  expected EOS");
        return -1;
    }
 endFunction:
    if (dateTime.tm_year<0) {
        dateTime.tm_wday=-1;
        dateTime.tm_yday=-1;
    } else{
        struct tm tmpt;
        tmpt=dateTime;
        mktime(&tmpt);
        dateTime.tm_wday=tmpt.tm_wday;
        dateTime.tm_yday=tmpt.tm_yday;
    }
    
    return 0;
}


/** Parse date-setup string and add specifiead word into string.
 * Input string contains month and day names and is formated as follows:
 * |January|February|...|December|Jan|Feb|...|Dec|Sunday|Monday|...|Saturday|Sun|Mon|...|Sat|
 * @return Status: 0=ok, -1=error.
 * @param index Which substring put from setup-string into output.
 * @param setup The date-setup string.
 * @param out output string. */
static int addDateString(unsigned int index, const string &setup,
                         string &out)
{
    // find the proper word
    string::const_iterator isetup = setup.begin();
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

    string::const_iterator esetup = isetup;
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

template <typename T1>
static inline void formatValue(string &out, const char *format,
                               T1 v1)
{
    char buf[60];
    unsigned int len = snprintf(buf, sizeof(buf), format, v1);
    out.append(buf, (len > sizeof(buf)) ? sizeof(buf) : len);
}

template <typename T1, typename T2>
static inline void formatValue(string &out, const char *format,
                               T1 v1, T2 v2)
{
    char buf[60];
    unsigned int len = snprintf(buf, sizeof(buf), format, v1, v2);
    out.append(buf, (len > sizeof(buf)) ? sizeof(buf) : len);
}

template <typename T1, typename T2, typename T3>
static inline void formatValue(string &out, const char *format,
                               T1 v1, T2 v2, T3 v3)
{
    char buf[60];
    unsigned int len = snprintf(buf, sizeof(buf), format, v1, v2, v3);
    out.append(buf, (len > sizeof(buf)) ? sizeof(buf) : len);
}

template <typename T1, typename T2, typename T3, typename T4>
static inline void formatValue(string &out, const char *format,
                               T1 v1, T2 v2, T3 v3, T4 v4)
{
    char buf[60];
    unsigned int len = snprintf(buf, sizeof(buf), format, v1, v2, v3, v4);
    out.append(buf, (len > sizeof(buf)) ? sizeof(buf) : len);
}

/** Function for formating dates like strftime() from struct tm.
 * @return Status: 0=ok, -1=error.
 * @param format Format string (like C strftime).
 * @param setup String with literals of month/day names.
 * @param dateTime Date/time for formating.
 * @param output Result (formated string). */
static int formatBrokenDate(const string &format, const string &setup,
                            const struct tm &dateTime, string &output)
{
    // for all chars in format string
    for (string::const_iterator ptr = format.begin();
         ptr != format.end(); ++ptr) {
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
static int formatStringDate(const string &format,const string &setup,
                            const string &date, string &output)
{
    struct tm dateTime;
    
    if (parseDateTime(date, dateTime)) return -1;
    return formatBrokenDate(format, setup, dateTime, output);
}

/** strftime for time_t since Epoch input 
 * @param format format string (like C strftime)
 * @param setup string for month/day names
 * @param date date to format
 * @param output result
 * @return 0 OK, -1 error
 * */
static int formatTime_tDate(const string &format, const string &setup,
                            time_t date, string &output)
{
    struct tm dateTime;
    if (!localtime_r(&date, &dateTime)) return -1;
    return formatBrokenDate(format, setup, dateTime, output);
}

/** strftime like Teng function. Uses user month / day names
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionFormatDate(const vector<ParserValue_t> &args,
                                  const Processor_t::FunctionParam_t &setting,
                                  ParserValue_t &result)
{
    ParserValue_t format;
    ParserValue_t date;
    ParserValue_t setup;

    switch (args.size()) {
    case 3:
        format = args[2];
        date = args[1];
        setup = args[0];
        break;

    case 2:
        format = args[1];
        date = args[0];
        setup.setString();
        break;

    default:
        result.setString("undefined");
        return -1;
    }

    date.validateThis();
    string res;
    if ((date.type == ParserValue_t::TYPE_INT) ||
        (date.type == ParserValue_t::TYPE_REAL)) {
        if (formatTime_tDate(format.stringValue, setup.stringValue,
                             date.integerValue, res)) {
            return -2;
        }
    } else {
        if (formatStringDate(format.stringValue, setup.stringValue,
                             date.stringValue, res)) {
            return -2;
        }
    }

    // OK
    result.setString(res);
    return 0;
}


/** Format number to be suitable for human reading.
 * Function works like round(), except it has two more params:
 * decimal point and thousands separator (both are strings). */
static int tengFunctionNumFormat(const vector<ParserValue_t> &args,
                                 const Processor_t::FunctionParam_t &setting,
                                 ParserValue_t &result)
{
    // powers of base=10
    static double pow10[] = {
        1.0e+0, 1.0e+1, 1.0e+2, 1.0e+3, 1.0e+4,
        1.0e+5, 1.0e+6, 1.0e+7, 1.0e+8, 1.0e+9,
        1.0e+10, 1.0e+11, 1.0e+12, 1.0e+13, 1.0e+14,
        1.0e+15, 1.0e+16, 1.0e+17, 1.0e+18, 1.0e+19,
        1.0e+20, 1.0e+21, 1.0e+22, 1.0e+23, 1.0e+24,
        1.0e+25, 1.0e+26, 1.0e+27, 1.0e+28, 1.0e+29,
        1.0e+30, 1.0e+31, 1.0e+32, 1.0e+33, 1.0e+34,
        1.0e+35, 1.0e+36, 1.0e+37, 1.0e+38, 1.0e+39
    };
    
    // prepare result
    result.setString("undefined");
    if ((args.size() < 2) || (args.size() > 4))
        return -1; //bad number of args
    
    // get args -- number and precission
    ParserValue_t a(args[args.size() - 1]); //number
    ParserValue_t b(args[args.size() - 2]); //prec
    // decimal point
    string decipoint = ".";
    if (args.size() >= 3)
        decipoint = args[args.size() - 3].stringValue;
    // thousands separator
    string thousandsep;
    if (args.size() >= 4)
        thousandsep = args[args.size() - 4].stringValue;
    
    // convert params to numbers
    a.validateThis();
    b.validateThis();
    if ((a.type == ParserValue_t::TYPE_STRING)
        || (b.type != ParserValue_t::TYPE_INT)
        || (b.integerValue > 39)
        || (b.integerValue < -39))
        return -2; //error
    
    // round the number
    double num = a.realValue;
    int sign = 1; // defaults to positive num
    if (num < 0) {
        sign = -1; // negative num
        num = -num;
    }
    
    double powernum = 0.0; //temp value for showing decimal part
    if (b.integerValue <= 0) {
        num /= pow10[-b.integerValue];
        num = round(num);
        num *= pow10[-b.integerValue];
    }
    else {
        num *= pow10[b.integerValue];
        num = round(num);
        powernum = num;
        num /= pow10[b.integerValue];
    }
    
    // split number into integer and decimal parts and
    // print string using thousand and decimal separators
    double integer = trunc(num);
    long long n = static_cast<long long>(integer);
    string str;
    char buf[16];
    int m;
    // if zero integer part
    if (n == 0) {
        // render '0'
        snprintf(buf, sizeof(buf), "%s0",
                (b.integerValue > 0 && sign < 0) ? "-" : "");
        str = buf;
    }
    else {
        // while something rest
        while (n > 0) {
            m = n % 1000; //thousand part
            n /= 1000; //shift
            if (n > 0) {
                snprintf(buf, sizeof(buf), "%03d", m);
                str = thousandsep + buf + str;
            }
            else {
                snprintf(buf, sizeof(buf), "%s%d", sign < 0 ? "-" : "", m);
                str = string(buf) + str;
            }
        }
    }
    // if decimal part
    if (b.integerValue > 0) {
        n = static_cast<long long>(powernum);
        string str2;
        int i;
        for (i = 0; i < b.integerValue; ++i) {
            m = n % 10; //decimal part
            n /= 10; //shift
            snprintf(buf, sizeof(buf), "%d", m);
            str2 = buf + str2;
        }
        str = str + decipoint + str2;
    }
    
    // store result
    result.setString(str);
    return 0; //ok
}


/** Round real number to a defined precision.
 * Examples: round(1234.56789, 2) => 1234.57, round(1234.56, -2) => 1200.
 * @param args Teng function arguments.
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionRound(const vector<ParserValue_t> &args,
                             const Processor_t::FunctionParam_t &setting,
                             ParserValue_t &result)
{
    static double pow10[] = {
        1.0e+0, 1.0e+1, 1.0e+2, 1.0e+3, 1.0e+4,
        1.0e+5, 1.0e+6, 1.0e+7, 1.0e+8, 1.0e+9,
        1.0e+10, 1.0e+11, 1.0e+12, 1.0e+13, 1.0e+14,
        1.0e+15, 1.0e+16, 1.0e+17, 1.0e+18, 1.0e+19,
        1.0e+20, 1.0e+21, 1.0e+22, 1.0e+23, 1.0e+24,
        1.0e+25, 1.0e+26, 1.0e+27, 1.0e+28, 1.0e+29,
        1.0e+30, 1.0e+31, 1.0e+32, 1.0e+33, 1.0e+34,
        1.0e+35, 1.0e+36, 1.0e+37, 1.0e+38, 1.0e+39
    };
    
    result.setString("undefined");
    if (args.size()!=2) return -1;
    ParserValue_t a(args[1]);
    ParserValue_t b(args[0]);
    
    a.validateThis();
    b.validateThis();
    if ((a.type == ParserValue_t::TYPE_STRING)
        || (b.type != ParserValue_t::TYPE_INT)
        || (b.integerValue > 39)
        || (b.integerValue < -39))
        return -2;
    
    if (a.type == ParserValue_t::TYPE_INT) {
        if (b.integerValue >= 0) {
            result.setInteger(a.integerValue);
            return 0;
        }
        int l, k, sign;
        k = a.integerValue;
        sign = k < 0 ? -1 : 1;
        for (l = 0; l > b.integerValue; --l)
            k = (k + sign * 5) / 10;
        for (l = 0; l > b.integerValue; --l)
            k *= 10;
        result.setInteger(k);
        return 0;
    }
    
    double f = a.realValue;
    if (b.integerValue <= 0) {
        f /= pow10[-b.integerValue];
        f = round(f);
        f *= pow10[-b.integerValue];
        result.setReal(f);
    }
    else {
        f *= pow10[b.integerValue];
        f = round(f);       
        f /= pow10[b.integerValue];
        result.setReal(f, b.integerValue);
    }
    
    return 0;
}

/** Makes Teng integer from Teng real.
 * @param args Teng function arguments
 * @param setting Teng function setting
 * @param result Teng function result
 * @return 0 OK, -1 wrong argument count, -2 other error
 * */
static int tengFunctionInt(const vector<ParserValue_t> &args,
                           const Processor_t::FunctionParam_t &setting,
                           ParserValue_t &result)
{
    result.setString("undefined");
    if (args.size() != 1) return -1;
    ParserValue_t a(args[0]);
    
    a.validateThis();
    if (a.type == ParserValue_t::TYPE_STRING) {
        setting.logger.logError(Error_t::LL_ERROR,
                "int(): Cannot convert string to int.");
        return -2;
    }
    result.setInteger(a.integerValue);
    return 0;
}


/** Escape disallowed chars in URL arguments.
 * @return Status: 0=ok, -1=wrong argument count, -2=other error.
 * @param args Function arguments (list of values).
 * @param setting Teng function setting.
 * @param result Function's result value. */
static int tengFunctionUrlEscape(const vector<ParserValue_t> &args,
                                 const Processor_t::FunctionParam_t &setting,
                                 ParserValue_t &result)
{
    // check params
    result.setString("undefined");
    if (args.size() != 1)
        return -1; //bad args
    ParserValue_t a(args[0]); //take 1st arg
    
    // quote
    string res;
    string::const_iterator i;
    for (i = a.stringValue.begin(); i != a.stringValue.end(); ++i) {
        if (isalnum(*i) || (*i == '_') || (*i == '-')
            || (*i == '.') || *i == '/') {
            res += *i;
        } else {
            char buf[16];
            snprintf(buf, sizeof(buf), "%%%02X",
                     static_cast<unsigned char>(*i));
            res += buf;
        }
    }
    // success
    result.setString(res);
    return 0;
}


/** Format long number for better readability.
 * @return Status: 0=ok, -1=wrong argument count, -2=other error.
 * @param args Function arguments (list of values).
 * @param setting Teng function setting.
 * @param result Function's result value. */
static int tengFunctionNL2BR(const vector<ParserValue_t> &args,
                             const Processor_t::FunctionParam_t &setting,
                             ParserValue_t &result)
{
    // check params
    result.setString("undefined");
    if (args.size() != 1)
        return -1; //bad args
    ParserValue_t str(args[0]); //take 1st arg
    
    // convert
    string res;
    for (string::const_iterator i = str.stringValue.begin();
         i != str.stringValue.end(); ++i) {
        if (*i == '\n') res += "\n<br />";
        else res += *i;
    }
    // success
    result.setString(res);
    return 0;
}


/** Check whether argument is a number.
 * @return Status: 0=ok, -1=wrong argument count, -2=other error.
 * @param args Function arguments (list of values).
 * @param setting Teng function setting.
 * @param result Function's result value. */
static int tengFunctionIsNumber(const vector<ParserValue_t> &args,
                                const Processor_t::FunctionParam_t &setting,
                                ParserValue_t &result)
{
    // check params
    result.setString("undefined");
    if (args.size() != 1)
        return -1; //bad args

    result.setInteger(args.front().type != ParserValue_t::TYPE_STRING);
    return 0;
}


/** Check whether argument is a number.
 * @return Status: 0=ok, -1=wrong argument count, -2=other error.
 * @param args Function arguments (list of values).
 * @param setting Teng function setting.
 * @param result Function's result value. */
static int tengFunctionSecToTime(const vector<ParserValue_t> &args,
                                 const Processor_t::FunctionParam_t &setting,
                                 ParserValue_t &result)
{
    // check params
    result.setString("undefined");
    if (args.size() != 1)
        return -1; //bad args
    
    ParserValue_t sec(args[0]);
    sec.validateThis();
    if (sec.type == ParserValue_t::TYPE_STRING)
        return -2; //not a number

    char buf[64];
    snprintf(buf, sizeof(buf), "%ld:%02ld:%02ld", sec.integerValue / 3600,
             (sec.integerValue % 3600) / 60, sec.integerValue % 60);

    result.setString(buf);
    return 0;
}


namespace {
    struct FunctionStub_t {
        char *name;        // teng name
        bool eval;         // use for preevaluation (false for rand(), time() etc)
        Function_t func;   // C++ function addr
    };

    static FunctionStub_t tengFunctions[] = {
        {"len", false, tengFunctionLen},          // like strlen in C
        {"random", false, tengFunctionRandom},    // random integer
        {"round", true, tengFunctionRound},       // round(number, precision)
        {"numformat", true, tengFunctionNumFormat}, // format number for display
        {"date", true, tengFunctionFormatDate},   // like strftime
        {"now", false, tengFunctionNow},          // like gettimeofday (returns real)
        {"substr", false, tengFunctionSubstr},    // like str[a:b] in Python
        {"wordsubstr", false, tengFunctionWordSubstr}, // like str[a:b] in Python
                                                        // but does not split words
        {"substr_word", false, tengFunctionWordSubstr}, // deprecated name
        {"escape", false, tengFunctionEscape},    // for example "<" => "&lt;"
        {"unescape", false, tengFunctionUnescape},// for example "&lt;" => "<"
        {"reorder", true, tengFunctionReorder},   // like sprintf with %s changing
                                                  // order
        {"int", true, tengFunctionInt},           // like (int) in C
        {"urlescape", true, tengFunctionUrlEscape}, // escape strange chars in urls
        {"nl2br", true, tengFunctionNL2BR},       // convert '\n' => <br />
        {"isnumber", true, tengFunctionIsNumber}, // checks whether argument is s number
        {"sectotime", true, tengFunctionSecToTime}, // convert seconds to HH:MM:SS
        {"sec_to_time", true, tengFunctionSecToTime}, // deprecated name
        { 0, false, 0}                            // end of list
    };
}

/** Find C++ function for Teng function.
 * @param name name of Teng function 
 * @param normalRun true if in runtime, false in template compiling
 *  optimalization (optimalizing strlen("hello") OK, now() error)
 * @return function OK, 0 error
 * */
Function_t Teng::tengFindFunction(const string &name, bool normalRun) {
    // try to find function
    for (FunctionStub_t *p = tengFunctions; p->name; ++p)
        if ((normalRun || p->eval) && (p->name == name))
            return p->func;

    // not found
    return 0;
}

