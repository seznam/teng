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
 * $Id: tengprocessor.h,v 1.1 2004-07-28 11:36:55 solamyl Exp $
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
 */

#ifndef _TENGPROCESSOR_H
#define _TENGPROCESSOR_H

#include <string>
#include <vector>
#include <map>

#include "tengerror.h"
#include "tengprogram.h"
#include "tengdictionary.h"
#include "tengstructs.h"
#include "tengformatter.h"
#include "tenginstruction.h"
#include "tengparservalue.h"
#include "tengcontenttype.h"

using namespace std;

namespace Teng {

/** item of fragment stack in Processor_t */
struct FragmentStackFrame_t {
    /** @short Current iteration.
     */
    int iteration;

    /** @short Indicates start of block.
     */
    bool stackLine;

    /** Where to jump on iteration end (after <?teng endfrag?>
     */
    int jmp;

    /** @ short Name of associated fragment.
     */
    string name;

    /** @short Number of iterations (varibale name._count).
     */
    ParserValue_t count;  // _count variable

    /** @short Current of iterations (varibale name._number).
     */
    ParserValue_t number;

    /** @short Local variables set from template.
     */
    map<string, ParserValue_t> vars;
};

typedef vector<FragmentStackFrame_t> OldFragmentStack_t;

class Processor_t {
public:
    
    /** Inititalize processor.
     * @param program Program in byte-code to interpret.
     * @param dict Language dictionary.
     * @param param Language-independent dictionaru (param.conf).
     * @param encoding template encoding
     * @param escaper output string escaper
     * */
    Processor_t(const Program_t *program, const Dictionary_t *dict,
                const Dictionary_t *param, const string &encoding,
                const ContentType_t &escaper, bool errorFragment = false);
    
    /** Execute program.
     * @param data Application data supplied bu user.
     * @param writer Output stream object.
     * @param error Error log object. */
    void run(const Fragment_t *data, Formatter_t *writer,
             Error_t *error);
    
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
                        const ContentType_t &escaper);
        
        string encoding;  /** < encoding of template
                              (other UTF-8 string functions) */
        const ContentType_t &escaper; /** < string escaping */
        mutable Logger_t logger;
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
   
    /** print all subtrtee with spaces from left
     *  @param f root
     *  @param prefix prefix at line start
     * */  
    int dumpFragment(const Fragment_t *f, const string &prefix = string());
    
    /** print all app data tree */ 
    int instructionDebug();
    
    /** program (translated template) */
    const Program_t *program;
    
    /** language specific dictionary */
    const Dictionary_t *langDictionary;
    
    /** param dictionary */
    const Dictionary_t *paramDictionary;
    
    /** data from application */
    const Fragment_t *data;
    
    /** std output object */
    Formatter_t *output;
    
    /** log error object */
    Error_t *error;
    
    /** processor stack */
    stack<ParserValue_t> valueStack; 
    
    /** fragment stack */
    OldFragmentStack_t oldFragmentStack; 
    
    /** fragment.name -> fragment.iteration */
    map<string,int> fragmentIter; 
    
    /** root fragment (specific, not on fragment stack) */
    FragmentStackFrame_t root;
    
    /** variable, that are added to teng user function (as 1 ptr to struct)*/
    FunctionParam_t fParam;
    
    /** is ._error special fragment? */
    bool errorFragment;
};
    
} // namespace Teng

#endif // _TENGPROCESSOR_H
