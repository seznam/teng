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
 * $Id: tengprocessor.h,v 1.2 2004-12-30 12:42:02 vasek Exp $
 *
 * DESCRIPTION
 * Teng processor. Executes programs.
 *
 * AUTHORS
 * Jan Nemec <jan.nemec@firma.seznam.cz>
 * Vaclav Blazek <blazek@firma.seznam.cz>
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
#include <map>

#include "tengerror.h"
#include "tengprogram.h"
#include "tengdictionary.h"
#include "tengconfiguration.h"
#include "tengstructs.h"
#include "tengformatter.h"
#include "tenginstruction.h"
#include "tengparservalue.h"
#include "tengcontenttype.h"

using namespace std;

namespace Teng {

class Processor_t {
public:
    
    /** Inititalize processor.
     * @param program Program in byte-code to interpret.
     * @param dict Language dictionary.
     * @param param Language-independent dictionaru (param.conf).
     * @param encoding template encoding
     * @param escaper output string escaper
     * */
    Processor_t(const Program_t &program, const Dictionary_t &dict,
                const Configuration_t &param, const string &encoding,
                const ContentType_t *contentType);
    
    /** Execute program.
     * @param data Application data supplied bu user.
     * @param writer Output stream object.
     * @param error Error log object. */
    void run(const Fragment_t &data, Formatter_t &writer,
             Error_t &error);
    
    /** Try to evaluate an expression.
     * @return 0=ok (expression evaluated), -1=error (cannot evaluate).
     * @param result Structure for output value in case of success.
     * @param startAddress Run program from this address.
     * @param endAddress Pointer after the last instruction of the prog. */
    int eval(ParserValue_t &result, int startAddress, int endAddress);

    class Logger_t {
    public:
        inline Logger_t(Processor_t &processor)
            : processor(processor), instr(0)
        {}

        void logError(Error_t::Level_t level, const string &message);

        inline void setInstruction(const Instruction_t *instr) {
            this->instr = instr;
        }

    private:
        Processor_t &processor;
        const Instruction_t *instr;
    };
    
    /** this structure must be added as param to all user teng functions
     *  used by string function. */
    struct FunctionParam_t {
        FunctionParam_t(Processor_t &processor, const string &encoding,
                        const ContentType_t *contentType,
                        const Configuration_t &configuration);
        
        string encoding;  /** < encoding of template
                              (other UTF-8 string functions) */
        Escaper_t escaper; /** < string escaping */
        mutable Logger_t logger;
        const Configuration_t &configuration;
    };

    // we have to access logErr method, otherwise unaccesible
    friend class Logger_t;

private:
    /** Logs runtime error
     * @param instr on which instruction
     * @param s error message */
    void logErr(const Instruction_t &instr, const string &s,
                Error_t::Level_t level);
    
    
    /** Logs runtime error where no instruction available
     * @param s error message */
    void logErrNoInstr(const string &s, Error_t::Level_t level);
    
    
    /** Evaluate simple binary numeric operation
     * @return -1 (error) or 0 (OK)
     * @param instr instruction
     * @param a temporary value
     * @param b temporary value
     * */
    int numOp(const Instruction_t &instr);
    
    /** Evaluate simple binary numeric operation in preevaluation
     * @return -1 (error) or 0 (OK)
     * @param instr instruction
     * @param a temporary value
     * @param b temporary value
     * */
    int evalNumOp(const Instruction_t &instr);
    
    /** Evaluate simple binary nonnumeric operation
     * @return -1 (error) or 0 (OK)
     * @param instr instruction
     * @param a temporary value
     * @param b temporary value
     * */   
    int binaryOp(const Instruction_t &instr);
    
    /** Evaluate simple binary nonnumeric operation in preevaluation
     * @return -1 (error) or 0 (OK)
     * @param instr instruction
     * @param a temporary value
     * @param b temporary value
     * */   
    int evalBinaryOp(const Instruction_t &instr);
   
    /** print all app data tree */ 
    int instructionDebug(const Fragment_t &data, Formatter_t &output);
    
    /** program (translated template) */
    const Program_t &program;
    
    /** language specific dictionary */
    const Dictionary_t &langDictionary;
    
    /** param dictionary */
    const Configuration_t &configuration;
    
    /** log error object */
    Error_t *error;
    
    /** processor stack */
    stack<ParserValue_t> valueStack; 
    
    /** fragment.name -> fragment.iteration */
    map<string,int> fragmentIter; 
    
    /** variable, that are added to teng user function (as 1 ptr to struct)*/
    FunctionParam_t fParam;
};
    
} // namespace Teng

#endif // TENGPROCESSOR_H
