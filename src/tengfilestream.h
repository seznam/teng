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
 * $Id: tengerror.h,v 1.6 2011-01-19 06:39:45 burlog Exp $
 *
 * DESCRIPTION
 * C++ stream writting to FILE *.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2018-06-14 (burlog)
 *            First draft.
 *
 */

#ifndef TENGFILESTREAM_H
#define TENGFILESTREAM_H

#include <string>
#include <cstring>
#include <ostream>

namespace Teng {

/** Standard streambuf for FILE *.
 */
class FileBuf_t: public std::streambuf {
public:
    FileBuf_t(FILE *file)
        : file(file)
    {}

protected:
    /** Called when the one char should be writen.
     */
    int_type overflow(int_type ch) override {
        return (ch == EOF) || (fwrite(&ch, 1, 1, file) != 1)
             ? EOF
             : ch;
    }

    /** Called when the char sequence should be writen.
     */
    std::streamsize xsputn(const char *s, std::streamsize size) override {
        auto written = fwrite(s, 1, size, file);
        return (written != size) && ferror(file)? 0: written;
    }

    FILE *file; //!< the C stream
};

/** C++ stream that writes to FILE *.
 */
class FileStream_t: public std::ostream {
public:
    FileStream_t(FILE *file): fileBuf(file) {rdbuf(&fileBuf);}

protected:
    FileBuf_t fileBuf; //!< the streambuf instance
};

} // namespace Teng

#endif // TENGFILESTREAM_H

