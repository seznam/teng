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
 * $Id: tengwriter.h,v 1.4 2006-05-19 07:30:44 vasek Exp $
 *
 * DESCRIPTION
 * Teng writer.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-17  (vasek)
 *             Created.
 */


#ifndef TENGWRITER_H
#define TENGWRITER_H

#include <string>
#include <cstdio>

#include <tengerror.h>

namespace Teng {

/** @short Output writer.
 */
class Writer_t {
public:
    // don't copy
    Writer_t(const Writer_t &) = delete;
    Writer_t &operator=(const Writer_t &) = delete;

    // types
    using StringSpan_t = std::pair<
        std::string::const_iterator, std::string::const_iterator
    >;

    /** @short Create new writer.
     */
    Writer_t(): err(nullptr) {}

    /** @short Destroy writer.
     */
    virtual ~Writer_t() = default;

    /** @short Sets new error log.
     */
    void setError(Error_t *new_err) {err = new_err;}

    /** @short Write given string to output.
     *  Abstract, must be overloaded in subclass.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const std::string &str) = 0;

    /** @short Write given string to output.
     *  Abstract, must be overloaded in subclass.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const char *str) = 0;

    /** @short Write given string to output.
     *  Abstract, must be overloaded in subclass.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const char *str, std::size_t size) = 0;

    /** @short Write given string to output.
     *  Abstract, must be overloaded in subclass.
     *  @param str string to be written
     *  @param interval iterators to given string, only this part
     *                  shall be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const std::string &str, StringSpan_t interval) = 0;

    /** @short Flush buffered data to the output.
     *  Abstract, must be overloaded in subclass.
     *  @return 0 OK, !0 error
     */
    virtual int flush() = 0;

    /** @short Write given string to output.
     *  @param istr begin of string to be written
     *  @param estr end of string to be written
     *  @return 0 OK, !0 error
     */
    int write(const char *istr, const char *estr) {
        return write(istr, estr - istr);
    }

protected:
    /** @short Error log.
     */
    Error_t *err;
};

/** @short Output writer. Writes to the associated string.
 */
class StringWriter_t : public Writer_t {
public:
    /** @short Creates new writer. Associates string.
     *  @param str output string
     */
    StringWriter_t(std::string &str);

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    int write(const std::string &str) override;

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    int write(const char *str) override;

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    int write(const char *str, std::size_t size) override;

    /** @short Write given string to output.
     *  @param str string to be written
     *  @param interval iterators to given string, only this part
     *                  shall be written
     *  @return 0 OK, !0 error
     */
    int write(const std::string &str, StringSpan_t interval) override;

    /** @short Flush buffered data to the output.
     *  No-op.
     *  @return 0 OK, !0 error
     */
    int flush() override { return 0; }

private:
    /** @short Associated string.
     */
    std::string &str;
};

/** @short Output writer. Writes to associated file.
 */
class FileWriter_t : public Writer_t {
public:
    /** @short Create new writer.
     *  @param filename file to open
     */
    FileWriter_t(const std::string &filename);

    /** @short Create new writer from open file.
     *  File is borrowed. It'll be not closed.
     *  @param file open file
     */
    FileWriter_t(FILE *file);

    /** @short Destroy writer.
     *  Associated file will be closed unles it's borrowed.
     */
    ~FileWriter_t() override;

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    int write(const std::string &str) override;

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    int write(const char *str) override;

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    int write(const char *str, std::size_t size) override;

    /** @short Write given string to output.
     *  @param str string to be written
     *  @param interval iterators to given string, only this part
     *                  shall be written
     *  @return 0 OK, !0 error
     */
    int write(const std::string &str, StringSpan_t interval) override;

    /** @short Flush buffered data to the output.
     *  @return 0 OK, !0 error
     */
    int flush() override;

private:
    /** @short Output file.
     */
    FILE *file;

    /** @short Indicates whether file is borrowed.
     */
    bool borrowed;
};

} // namespace Teng

#endif // TENGWRITER_H

