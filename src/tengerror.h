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
 * $Id: tengerror.h,v 1.3 2006-06-21 14:13:59 sten__ Exp $
 *
 * DESCRIPTION
 * Teng error handling class.
 *
 * AUTHORS
 * Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (vasek)
 *             Created.
 * 2006-06-21  (sten__)
 *             Removed error duplicities.
 */

#ifndef TENGERROR_H
#define TENGERROR_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>

#include <errno.h>

namespace Teng {

/** @short Error handling class -- logs errors. */
class Error_t {
	
public:
    /** @short Level of message */
    enum Level_t {
        LL_DEBUG =   0, /** < Level for debug messages. */
        LL_WARNING = 1, /** < Level for warnings. */
        LL_ERROR =   2, /** < Level for error messages. */
        LL_FATAL =   3  /** < Level for fatal messages. */
    };

    /** @short Creates new error logger. */
    Error_t()
        : level(LL_DEBUG), entries()
    {};

    /** @short Holds position in file. */
    struct Position_t {
        /** @short Creates new position object.
         *  If lineno<=0 or col<0, then position within file is not printed.
         *  @param filename Name of the associated file.
         *  @param lineno Line number (starting with 1).
         *  @param col Column on line (starting with 0). */
        Position_t(const std::string &filename = "",
                   int lineno = 0, int col = 0)
            : filename(filename), lineno(lineno), col(col)
        {}

        /** @short Advances to the beginning of new line. */
        void newLine() {
            ++lineno;
            col = 0;
        }

        /** @short Advances column by given offset.
          * @param offset column offset */
        void advanceColumn(int offset = 1) {
            col += offset;
        }

        /** @short Advances column to the next tab position
          *        assuming that <TAB> is 8 chars */
        void advanceToTab() {
            col = 8 * (col / 8 + 1);
        }

        /** @short Advances position by given char.
         *  <LF> advances line, <TAB> advances to next tab
         *  and other char advances to nex column.
         *  @param c character
         */
        void advance(char c) {
            switch (c) {
            case '\n':
                newLine();
                break;
            case '\t':
                advanceToTab();
                break;
            default:
                ++col;
                break;
            }
        }

        /** @short Advances position to the end of given string.
         *  @param str string
         */
        void advance(const std::string &str) {
            for (std::string::const_iterator istr = str.begin();
                 istr != str.end(); )
                advance(*istr++);
        }
        
        /** @short Advances position to the end of given string.
         *  @param str string
         *  @param length length of string
         */
        void advance(const char *str, int length) {
            const char *end = str + length;
            for (const char *istr = str; istr != end; )
                advance(*istr++);
        }

        /** @short Explicitly sets column value.
          * @param col new column value */
        void setColumn(int col) {
            this->col = col;
        }

        /** @short Associated filename. */
        std::string filename;

        /** @short Line number. */
        int lineno;

        /** @short Column position in file. */
        int col;
    };
    
   /** Max level of messages (or LL_DEBUG if no message) */
    Level_t level;

    /** @short Entry in error log. */
    struct Entry_t {

        /** @short Creates new entry.
          * @param filename associated file
          * @param lineno line number
          * @param col column position in file
          * @param message additional message  */
        Entry_t(Level_t level, const std::string &filename, int lineno,
                int col, const std::string &message)
            : level(level), pos(filename, lineno, col), message(message)
        {}

        /** @short Creates new entry.
          * @param pos position in file
          * @param message column position in file */
        Entry_t(Level_t level, const Position_t &pos,
                const std::string &message)
            : level(level), pos(pos), message(message)
        {}

        /** @short Composes log line.
          * @return composed log line */
        std::string getLogLine() const {
            /** @short description of level*/
            static const std::string levelString[] =
                {"Debug", "Warning", "Error", "Fatal"};
            std::ostringstream out;
            // if 'filename' not given
            if (pos.filename.length() > 0)
                out << pos.filename;
            else
                out << "(no file)";
            // if 'lineno' and 'col' are positive
            if (pos.lineno > 0 && pos.col >= 0)
                out << "(" << pos.lineno << "," << pos.col << ")";
            out << " " << levelString[level];
            // add message and EOL
            out << ": " << message << std::endl;
            return out.str();
        }
       /** @short Level of message. */
        Level_t level;
        
        /** @short Position in file. */
        Position_t pos;
        
        /** @short Additional message. */
        std::string message;
    };

    /** @short Logs new error.
      * @param pos position in file
      * @param message additional message */
    void logError(Level_t level, const Position_t &pos,
                  const std::string &message) {
        for(unsigned int i = 0; i < entries.size(); i++) {
            if(entries[i].level == level && entries[i].pos.filename == pos.filename && entries[i].pos.lineno == pos.lineno && entries[i].pos.col == pos.col && entries[i].message == message) {
                return;
            }
        }
        entries.push_back(Entry_t(level, pos, message));
        if (level > this->level) this->level = level;
    }

    /** @short Log syscall error, no file associated. */
    void logSyscallError(Level_t level) {
        std::string strerr(strerror(errno));
        logError(level, Position_t("", -1, -1), "System call error: " + strerr);
    }

    /** @short Logs syscall error for given file/position.
      * @param pos position in file  */
    void logSyscallError(Level_t level, const Position_t &pos) {
        std::string strerr(strerror(errno));
        logError(level, pos, "System call error: " + strerr);
    }

    /** @short Logs syscall error for given file/position.
      * @param pos position in file
      * @param message additional message  */
    void logSyscallError(Level_t level, const Position_t &pos,
                         const std::string &message) {
        std::string strerr(strerror(errno));
        logError(level, pos, message + " (" + strerr + ")");
    }

    /** @short Logs syscall error for given file/position.
      * @param pos position in file
      * @param message additional message */
    void logRuntimeError(Level_t level, const Position_t &pos,
                         const std::string &message) {
        logError(level, pos, "Runtime: " + message);
    }
    
    /** @short Returns number of errors in log.
      * @return number of errors */
    int count() const {
        return entries.size();
    }
    /** @short Returns level variable.
      */
    int getLevel() const {
        return level;
    }
    /** @short Returns whether any error occurred.
      * @return true if any error occurred, false otherwise */
    operator bool() const {
        return !entries.empty();
    }

    /** @short Clears error log. */
    void clear() {
        entries.clear();
    }

    /** @short Appends content of another error log.
      * @param err appended log */
    void append(const Error_t &err) {
        // increase level if lower than that of err
        if (err.level > level) level = err.level;
        // append error log
        entries.insert(entries.end(), err.entries.begin(),
                       err.entries.end());
    }

    /** @short Get raw error log.
      * @return error log */
    const std::vector<Entry_t>& getEntries() const {
        return entries;
    }

    /** @short Dumps log into stream.
      * @param out output stream */
    void dump(std::ostream &out) const {
        for (std::vector<Entry_t>::const_iterator
                 ientries = entries.begin();
             ientries != entries.end(); ++ientries) {
            out << ientries->getLogLine();
        }
    }


private:
		
    /** @short Copy constructor intentionally private -- copying
     *         disabled. */
    Error_t(const Error_t&);

    /** @short Assignment operator intentionally private -- assignment
     *         disabled. */
    Error_t operator=(const Error_t&);

    /** @short Log of error entries. */
    std::vector<Entry_t> entries;
};

} // namespace Teng

#endif // TENGERROR_H
