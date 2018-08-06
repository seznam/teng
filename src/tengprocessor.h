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
 * $Id: tengprocessor.h,v 1.3 2005-04-26 13:56:25 vasek Exp $
 *
 * DESCRIPTION
 * Teng processor. Executes programs.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 * 2003-09-22  (jan)
 *             Created.
 * 2004-05-30  (vasek)
 *             Revised processor source code.
 * 2004-09-19  (vasek)
 *             Some minor fixes.
 */

#ifndef TENGPROCESSOR_H
#define TENGPROCESSOR_H

#include <string>

namespace Teng {

// forwards
class Error_t;
class Program_t;
class Fragment_t;
class Formatter_t;
class Dictionary_t;
class Configuration_t;
class ContentType_t;
namespace Parser {class Value_t;}

/** Does the template interpretation.
 */
class Processor_t {
public:
    /** Inititalize processor.
     *
     * @param err Error log object.
     * @param program Program in byte-code to interpret.
     * @param dict Language dictionary.
     * @param params Language-independent dictionaru (param.conf).
     * @param encoding Template encoding.
     * @param contentType Content type of template.
     */
    Processor_t(
        Error_t &err,
        const Program_t &program,
        const Dictionary_t &dict,
        const Configuration_t &params,
        const std::string &encoding = {},
        const ContentType_t *contentType = nullptr
    );

    /** Execute program.
     *
     * @param data Application data supplied by user.
     * @param writer Output stream object.
     */
    void run(const Fragment_t &data, Formatter_t &writer);

    /** Try to evaluate an expression.
     *
     * @param result Structure for output value in case of success.
     * @param startAddress Run program from this address.
     * @param endAddress Pointer after the last instruction of the prog.
     *
     * @return 0=ok (expression evaluated), -1=error (cannot evaluate).
     */
    int eval(Parser::Value_t &result, int start, int end);

protected:
    Error_t &err;                     //!< error log object
    const Program_t &program;         //!< program (translated template)
    const Dictionary_t &dict;         //!< language specific dictionary
    const Configuration_t &params;    //!< param dictionary
    const std::string &encoding;      //!< the template charset
    const ContentType_t *contentType; //!< the template content/mime type
};

} // namespace Teng

#endif // TENGPROCESSOR_H

