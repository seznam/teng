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
 * Teng regular expression class.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-11-20  (burlog)
 *             First version.
 */

#ifndef TENG_TENGREGEX_H
#define TENG_TENGREGEX_H

#define PCRE2_CODE_UNIT_WIDTH 8

#include <string>
#include <memory>
#include <stdexcept>
#include <vector>

#include <pcre2.h>

namespace Teng {

// forwards
class Regex_t;

/** Class that contains error destription.
 */
struct RegexError_t {
    using size_type = PCRE2_SIZE;
    using string_type = PCRE2_UCHAR8 *;

    /** Convers error code to error message.
     */
    static std::string message(int code) {
        std::string message(256, '\0');
        auto *buffer = reinterpret_cast<PCRE2_UCHAR8 *>(&message[0]);
        int result = pcre2_get_error_message(code, buffer, message.size());
        switch (result) {
        case PCRE2_ERROR_NOMEMORY:
            return message; // all bytes of message has been used
        case PCRE2_ERROR_BADDATA:
            return "invalid pcre2 error code";
        default:
            message.resize(result);
            break;
        }
        return message;
    }

    const char *regex; //!< the regex that is subject of the error
    size_type offset;  //!< where in regex is the error
    int code;          //!< the error code
};

/** The exception that is thrown when desired regular expression can't be
 * compiled.
 */
class RegexCompilationFailed_t: public std::runtime_error {
public:
    RegexCompilationFailed_t(const RegexError_t &error)
        : std::runtime_error(
            std::string("regular expression compilation failed")
            + ": regex=" + error.regex
            + ", error-offset=" + std::to_string(error.offset)
            + ", error-code=" + std::to_string(error.code)
            + ", error-msg=" + error.message(error.code)
        )
    {}
};

/** The exception that is thrown when pcre2_pattern_info failed.
 */
class RegexPatternInfoFailed_t: public std::runtime_error {
public:
    RegexPatternInfoFailed_t(int error_code)
        : std::runtime_error(
            std::string("the pcre2_pattern_info() function failed")
            + ": error-code=" + std::to_string(error_code)
            + ", error=" + RegexError_t::message(error_code)
        )
    {}
};

/** The exception that is thrown when JIT compilation failed.
 */
class RegexJitCompilationFailed_t: public std::runtime_error {
public:
    RegexJitCompilationFailed_t(int error_code)
        : std::runtime_error(
            std::string("regex JIT compilation failed")
            + ": error-code=" + std::to_string(error_code)
            + ", error=" + RegexError_t::message(error_code)
        )
    {}
};

/** The exception that is thrown when match failed.
 */
class RegexMatchFailed_t: public std::runtime_error {
public:
    RegexMatchFailed_t(int error_code)
        : std::runtime_error(
            std::string("matching pattern failed")
            + ": error-code=" + std::to_string(error_code)
            + ", error=" + RegexError_t::message(error_code)
        )
    {}
};

/** The exception that is thrown when match failed.
 */
class RegexNotImplementedYet_t: public std::runtime_error {
public:
    RegexNotImplementedYet_t(std::string message)
        : std::runtime_error(std::move(message))
    {}
};

/** The exception that is thrown when match failed.
 */
class RegexCapturesVectorIsTooSmall_t: public std::runtime_error {
public:
    RegexCapturesVectorIsTooSmall_t()
        : std::runtime_error(
            "the pcre2_match function returns zero; "
            "the number of capture groups does not fit "
            "the size of the captures vector"
        )
    {}
};

/** Represents capture/group in one match. Is valid untill next match will be
 * made.
 */
class RegexCapture_t {
public:
    // types
    using string_type = PCRE2_SPTR8;
    using size_type = PCRE2_SIZE;
    using const_iterator = const char *;

    /** C'tor.
     */
    RegexCapture_t(size_type start_i, const_iterator ipos, const_iterator epos)
        : start_i(start_i), ipos(ipos), epos(epos)
    {}

    /** Returns offset of capture in subject of matching.
     */
    size_type offset() const {return start_i;}

    /** Returns pointer to the first character of capture.
     */
    const_iterator begin() const {return ipos;}

    /** Returns pionter one past the last character of capture.
     */
    const_iterator end() const {return epos;}

    /** Returns the number of characters in capture.
     */
    size_type size() const {return std::distance(ipos, epos);}

    /** Returns captured character interval as a string.
     */
    std::string str() const {return {begin(), end()};}

protected:
    size_type start_i;   //!< offset of capture start in subject
    const_iterator ipos; //!< pointer to the first character of capture
    const_iterator epos; //!< pionter one past the last character of capture
};

/** This class represents one match of regular expression.
 */
class RegexMatch_t {
public:
    // types
    using size_type = PCRE2_SIZE;
    using match_free_type = decltype(&pcre2_match_data_free);
    using match_type = std::unique_ptr<pcre2_match_data, match_free_type>;
    using code_pointer = pcre2_code *;
    using string_type = PCRE2_SPTR8;
    using options_type = uint32_t;

    /** C'tor: forced no match.
     */
    RegexMatch_t(const Regex_t *re)
        : re(re), subject(), captures(nullptr), captures_size(0),
          match_data(nullptr, pcre2_match_data_free)
    {}

    /** C'tor: attempts make a match.
     */
    RegexMatch_t(
        const Regex_t *re,
        std::string subject,
        options_type options = {},
        size_type i = 0
    ): re(re), subject(std::move(subject)), captures(nullptr),
       captures_size(0), match_data(make_match(re))
    {do_match(options, i);}

    /** C'tor: move.
     */
    RegexMatch_t(RegexMatch_t &&other) noexcept
        : re(other.re), subject(std::move(other.subject)),
          captures(other.captures), captures_size(other.captures_size),
          match_data(std::move(other.match_data))
    {}

    /** Assigment: move.
     */
    RegexMatch_t &operator=(RegexMatch_t &&other) noexcept {
        if (this != &other) {
            subject = std::move(other.subject);       // can throw
            match_data = std::move(other.match_data); // don't throw
            re = other.re;                            // don't throw
            captures = other.captures;                // don't throw
            captures_size = other.captures_size;      // don't throw
        }
        return *this;
    }

    /** Returns true if match has been made.
     */
    explicit operator bool() const {return !!match_data;}

    /** Returns the number of captures/groups.
     */
    size_type capture_count() const {return captures_size;}

    /** Returns i-th capture.
     */
    RegexCapture_t operator[](size_type i) const {
        return {
            captures[2 * i],
            subject.c_str() + captures[2 * i],
            subject.c_str() + captures[2 * i + 1]
        };
    }

    /** Returns next match that follows this one.
     */
    RegexMatch_t next() const;

protected:
    /** Returns next match that follows empty match.
     */
    RegexMatch_t next_after_empty(size_type next_start_i) const;

    /** Returns next match that follows non empty match.
     */
    RegexMatch_t next_after_non_empty(size_type next_start_i) const;

    /** Named c'tor for creating the first match.
     */
    static match_type make_match(code_pointer code) {
        return {
            pcre2_match_data_create_from_pattern(code, nullptr),
            pcre2_match_data_free
        };
    }

    /** Named c'tor for creating the first match.
     */
    static match_type make_match(const Regex_t *re);

    /** Does the match.
     */
    int do_match(code_pointer code, options_type options, size_type i) {
        options |= PCRE2_NO_UTF_CHECK;
        return pcre2_match(
            code,
            reinterpret_cast<string_type>(subject.c_str()),
            PCRE2_ZERO_TERMINATED,
            i,
            options,
            match_data.get(),
            nullptr
        );
    }

    /** Does the match.
     */
    void do_match(options_type options, size_type i);

    const Regex_t *re;       //!< pointer to the compiled code
    std::string subject;     //!< the subject of matching
    size_type *captures;     //!< vector of capture groups
    size_type captures_size; //!< the number of capures
    match_type match_data;   //!< represents the current match
};

/** Represents compiled regular expression prepared to do things.
 */
class Regex_t {
public:
    // types
    using size_type = PCRE2_SIZE;
    using code_free_type = decltype(&pcre2_code_free);
    using code_type = std::unique_ptr<pcre2_code, code_free_type>;
    using string_type = PCRE2_SPTR8;
    using options_type = uint32_t;
    enum class endl {crlf, lf, cr, any, anycrlf, nul};

    /** C'tor.
     */
    Regex_t(const std::string &re)
        : pattern_string(re), code(compile(re)),
          endl_value(detect_endl(code))
    {}

    /** Returns what kind of endline is used in pattern.
     */
    endl endline() const {return endl_value;}

    /** Returns first match.
     */
    RegexMatch_t match(const std::string &subject) const {
        return RegexMatch_t(this, subject);
    }

    /** Replaces each captured group with string in with. Support references to
     * captures with shell like syntax - ${n} where n is a number of capture.
     */
    std::string replace(const std::string &subject, const std::string &with) {
        std::string result;
        result.reserve(subject.size());

        // precalculate references from with to captures/groups
        auto references = make_replace_referencies(with);

        // replace all matches with with string
        std::size_t subject_offset = 0;
        if (auto match_value = match(subject)) do {
            // use overall pattern capture (it is at index 0)
            auto capture = match_value[0];

            // if there are some chars before the match then copy them to result
            auto prefix_size = capture.offset() - subject_offset;
            if (prefix_size > 0)
                result.append(subject, subject_offset, prefix_size);

            // then replace capture with expanded with string
            expand_referencies(match_value, references, with, result);

            // move one character past the capture
            subject_offset = capture.offset() + capture.size();

        } while ((match_value = match_value.next()));

        // if there is some tail then append it to the result
        auto suffix_size = subject.size() - subject_offset;
        if (suffix_size > 0)
            result.append(subject, subject_offset, suffix_size);

        // done
        return result;
    }

protected:
    // need access code
    friend class RegexMatch_t;

    /** Represents the reference to capture in replace with string.
     */
    struct ReplaceReferencies_t {
        ReplaceReferencies_t(const RegexCapture_t &capture)
            : offset_value(capture.offset()), size_value(capture.size()),
              id_value(std::atoi(capture.begin() + 1)) // skip $
        {}

        // accessors
        size_type offset() const {return offset_value;}
        size_type size() const {return size_value;}
        uint32_t id() const {return id_value;}

    protected:
        size_type offset_value; //!< the offset of reference in with string
        size_type size_value;   //!< the size of reference in with string
        uint32_t id_value;      //!< the reference id
    };

    /** Creates an array of references from with to captured parts of subject.
     */
    std::vector<ReplaceReferencies_t>
    make_replace_referencies(const std::string &with) {
        static const Regex_t refs_re("\\$\\d+");
        std::vector<ReplaceReferencies_t> references;
        if (auto refs_match = refs_re.match(with)) do {
            references.push_back(refs_match[0]);
        } while ((refs_match = refs_match.next()));
        return references;
    }

    /** Appends expanded references in with string at the end of result string.
     */
    void expand_referencies(
        const RegexMatch_t &match_value,
        const std::vector<ReplaceReferencies_t> &references,
        const std::string &with,
        std::string &result
    ) {
        std::size_t with_offset = 0;

        // process all references
        for (auto &ref: references) {
            // if there are characters before the match then copy them to result
            auto prefix_size = ref.offset() - with_offset;
            if (prefix_size > 0)
                result.append(with, with_offset, prefix_size);

            // append the reference value
            if (ref.id() < match_value.capture_count()) {
                auto capture = match_value[ref.id()];
                result.append(capture.begin(), capture.end());
            }

            // move one character past the capture
            with_offset = ref.offset() + ref.size();
        }

        // if there is some tail then append it to the result
        auto suffix_size = with.size() - with_offset;
        if (suffix_size > 0)
            result.append(with, with_offset, suffix_size);
    }

    /** Detects what kind of endline is used in pattern.
     */
    static endl detect_endl(code_type &code) {
        uint32_t result;
        static const auto endl_info = PCRE2_INFO_NEWLINE;
        if (auto res_code = pcre2_pattern_info(code.get(), endl_info, &result))
            throw RegexPatternInfoFailed_t(res_code);
        switch (result) {
        case PCRE2_NEWLINE_CR: return endl::cr;
        case PCRE2_NEWLINE_LF: return endl::lf;
        case PCRE2_NEWLINE_CRLF: return endl::crlf;
        case PCRE2_NEWLINE_ANY: return endl::any;
        case PCRE2_NEWLINE_ANYCRLF: return endl::anycrlf;
#ifdef PCRE2_NEWLINE_NUL
        case PCRE2_NEWLINE_NUL: return endl::nul;
#endif
        }
        throw std::runtime_error(__PRETTY_FUNCTION__);
    }

    /** Named code c'tor thath makes things shorter.
     */
    static code_type
    compile(
        const std::string &pattern,
        options_type options,
        RegexError_t &error
    ) {
        options |= PCRE2_UTF;
        return {
            pcre2_compile(
                reinterpret_cast<string_type>(pattern.c_str()),
                PCRE2_ZERO_TERMINATED,
                options,
                &error.code,
                &error.offset,
                nullptr
            ), pcre2_code_free
        };
    }

    /** DOes jit compilation if it is allowed.
     */
    static void jit_compile(code_type &code) {
        options_type options = PCRE2_JIT_COMPLETE;
        if (auto res_code = pcre2_jit_compile(code.get(), options))
            throw RegexJitCompilationFailed_t(res_code);
    }

    /** Compiles regular expression and returns compiled pcre2 native code.
     */
    static code_type compile(const std::string &pattern) {
        options_type options = {};
        RegexError_t error = {pattern.c_str(), 0, 0};

        // compile the regex to pcre2 native code
        if (auto code = compile(pattern, options, error)) {
            jit_compile(code);
            return code;
        }
        throw RegexCompilationFailed_t(error);
    }

    std::string pattern_string; //!< the pattern as a sgring
    code_type code;             //!< the pcre2 native code of the regular expr
    endl endl_value;            //!< what kind of endline is used in pattern
};

RegexMatch_t::match_type RegexMatch_t::make_match(const Regex_t *re) {
    if (auto match_data = make_match(re->code.get()))
        return match_data;
    throw std::bad_alloc();
}

void RegexMatch_t::do_match(options_type options, size_type i) {
    switch (auto res_code = do_match(re->code.get(), options, i)) {
    case PCRE2_ERROR_NOMATCH:
        return match_data.reset();

    case 0:
        throw RegexCapturesVectorIsTooSmall_t();

    default:
        if (res_code < 0) throw RegexMatchFailed_t(res_code);
        captures = pcre2_get_ovector_pointer(match_data.get());
        captures_size = res_code;
        if (captures[0] > captures[1])
            throw RegexNotImplementedYet_t("\\K is not supported yet");
        break;
    }
}

RegexMatch_t RegexMatch_t::next() const {
    // previous (this) match
    size_type prev_start_i = captures[0];
    size_type prev_end_i = captures[1];

    // next (returned) match
    size_type next_start_i = prev_end_i;

    // prepare start offset
    return (prev_start_i == prev_end_i)
        ? next_after_empty(next_start_i)
        : next_after_non_empty(next_start_i);
}

RegexMatch_t RegexMatch_t::next_after_empty(size_type next_start_i) const {
    // if we are at the end of subject then return no match
    if (next_start_i == subject.size())
        return {re};

    // tell the pcre2 engine that we need non empty match and match it
    options_type options = PCRE2_NOTEMPTY_ATSTART | PCRE2_ANCHORED;
    if (auto match = RegexMatch_t(re, subject, options, next_start_i))
        return match;

    // there is no non empty match so move to next character (ascii)
    next_start_i += 1;

    // respect two-byte endlines
    switch (re->endline()) {
    case Regex_t::endl::any:
    case Regex_t::endl::crlf:
    case Regex_t::endl::anycrlf:
        // move past the new line
        if (next_start_i < (subject.size() - 1))
            if (subject[next_start_i] == '\r')
                if (subject[next_start_i + 1] == '\n')
                    return RegexMatch_t(re, subject, {}, next_start_i + 1);
        [[fallthrough]];
    default:
        // respect utf-8: move to next utf-8 character
        for (; next_start_i < subject.size(); ++next_start_i)
            if ((subject[next_start_i] & 0xc0) != 0x80)
                break;
        break;
    }
    return RegexMatch_t(re, subject, {}, next_start_i);
}

RegexMatch_t RegexMatch_t::next_after_non_empty(size_type next_start_i) const {
    // the \K moves start index before real match index
    auto real_prev_start_i = pcre2_get_startchar(match_data.get());
    if (next_start_i <= real_prev_start_i) {
        // if we are past the end of subect return no match
        if (real_prev_start_i >= subject.size())
            return {re};

        // jump to next character (ascii)
        next_start_i = real_prev_start_i + 1;

        // respect utf-8: move to next utf-8 character
        for (; next_start_i < subject.size(); ++next_start_i)
            if ((subject[next_start_i] & 0xc0) != 0x80)
                break;
    }

    // and do next match
    return RegexMatch_t(re, subject, {}, next_start_i);
}

} // namespace Teng

#endif /* TENG_TENGREGEX_H */
