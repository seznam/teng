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
 * $Id: tengwriter.h,v 1.2 2004-12-30 12:42:02 vasek Exp $
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
#include <stdio.h>

#include <tengerror.h>

namespace Teng {

/** @short Output writer.
 */
class Writer_t {
public:
    /** @short Create new writer.
     */
    inline Writer_t()
        : err()
    {}

    /** @short Destroy writer.
     */
    inline virtual ~Writer_t() {}

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
     *  @param interval iterators to given string, only this part
     *                  shall be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const std::string &str,
                      std::pair<std::string::const_iterator,
                      std::string::const_iterator> interval) = 0;

    /** @short Flush buffered data to the output.
     *  Abstract, must be overloaded in subclass.
     *  @return 0 OK, !0 error
     */
    virtual int flush() = 0;

    /** @short Get error log.
     *  @return error log
     */
    const Error_t& getErrors() const {
        return err;
    }

protected:
    /** @short Error log.
     */
    Error_t err;
    
private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    Writer_t(const Writer_t&);

    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    Writer_t operator=(const Writer_t&);
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
    virtual int write(const std::string &str);

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const char *str);

    /** @short Write given string to output.
     *  @param str string to be written
     *  @param interval iterators to given string, only this part
     *                  shall be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const std::string &str,
                      std::pair<std::string::const_iterator,
                      std::string::const_iterator> interval);

    /** @short Flush buffered data to the output.
     *  No-op.
     *  @return 0 OK, !0 error
     */
    virtual int flush() { return 0; }

private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    StringWriter_t(const StringWriter_t&);

    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    StringWriter_t operator=(const StringWriter_t&);

    /** @short Associated string.
     */
    std::string &str;
};

/** @short 
 *  @param 
 *  @return 
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
    virtual ~FileWriter_t();

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const std::string &str);

    /** @short Write given string to output.
     *  @param str string to be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const char *str);

    /** @short Write given string to output.
     *  @param str string to be written
     *  @param interval iterators to given string, only this part
     *                  shall be written
     *  @return 0 OK, !0 error
     */
    virtual int write(const std::string &str,
                      std::pair<std::string::const_iterator,
                      std::string::const_iterator> interval);


    /** @short Flush buffered data to the output.
     *  @return 0 OK, !0 error
     */
    virtual int flush();

private:
    /**
     * @short Copy constructor intentionally private -- copying
     *        disabled.
     */
    FileWriter_t(const StringWriter_t&);

    /**
     * @short Assignment operator intentionally private -- assignment
     *        disabled.
     */
    FileWriter_t operator=(const StringWriter_t&);

    /** @short Output file.
     */
    FILE *file;

    /** @short Indicates whether file is borrowed.
     */
    bool borrowed;
};

} // namespace Teng

#endif // TENGWRITER_H
