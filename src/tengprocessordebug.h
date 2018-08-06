    //
    // #<{(|* Evaluate simple binary nonnumeric operation in preevaluation
    //  * @return -1 (error) or 0 (OK)
    //  * @param instr instruction
    //  * @param a temporary value
    //  * @param b temporary value
    //  * |)}>#
    // int evalBinaryOp(const Instruction_t &instr);
    //
    // #<{(|* print all app data tree |)}>#
    // int instructionDebug(const Fragment_t &data, Formatter_t &output);
    //
// int dumpFragment(const Configuration_t &configuration,
//                  const Escaper_t &escaper,
//                  Formatter_t &output, const Fragment_t &fragment,
//                  const std::string &padding = std::string())
// {
//     // dump all variables (no nestedFragments)
//     for (Fragment_t::const_iterator ifragment = fragment.begin();
//          ifragment != fragment.end(); ++ifragment) {
//         if (!ifragment->second->getNestedFragments()) {
//             if (output.write(padding)) return -1;
//             if (output.write(ifragment->first)) return -1;
//             if (output.write(escaper.escape(": \""))) return -1;
//
//             // clip string to specified length
//             std::string strVal = *ifragment->second->getValue();
//             int unsigned len = configuration.getMaxDebugValLength();
//             if (len > 0)
//                 Teng::clipString(strVal, len);
//
//             if (output.write
//                 (escaper.escape(strVal + "\"\n")))
//                 return -1;
//
//         }
//     }
//
//     // dump all fragments (nestedFragments non-null)
//     for (Fragment_t::const_iterator ifragment = fragment.begin();
//          ifragment != fragment.end(); ++ifragment) {
//         if (ifragment->second->getNestedFragments()) {
//             unsigned int k = 0;
//             for (auto &nested: *ifragment->second->getNestedFragments()) {
//                 if (output.write(padding)) return -1;
//
//                 char s[20];
//                 if (output.write(ifragment->first)) return -1;
//                 sprintf(s, "[%u]: \n", k++);
//
//                 if (output.write(escaper.escape(s))) return -1;
//                 if (dumpFragment(configuration, escaper, output, *nested,
//                                  padding + "    "))
//                     return -1;
//                 if (output.write(escaper.escape("\n"))) return -1;
//             }
//         }
//     }
//
//     // OK
//     return 0;
// }
//
// int dumpBytecode(const Escaper_t &escaper, const Program_t &program,
//                  Formatter_t &output)
// {
//     // create bytecode dump
//     std::ostringstream os;
//     for (Program_t::const_iterator iprogram = program.begin();
//          iprogram != program.end(); ++iprogram) {
//         os << "0x" << std::hex << std::setw(8) << std::setfill('0')
//            << static_cast<unsigned int>(iprogram - program.begin()) << " ";
//         iprogram->dump(os, static_cast<unsigned int>(iprogram - program.begin()));
//     }
//
//     // write to output
//     return output.write(escaper.escape(os.str()));
// }
// int Processor_t::instructionDebug(const Fragment_t &data, Formatter_t &output) {
//     output.write(escaper.escape("Template sources:\n"));
//     const SourceList_t &pl = program.getSources();
//     for (unsigned int i = 0; i != pl.size(); ++i) {
//         if (output.write(escaper.escape("    " + pl.getSource(i) + "\n")))
//         return -1;
//     }
//
//     output.write(escaper.escape("\nLanguage dictionary sources:\n"));
//     const SourceList_t &l = langDictionary.getSources();
//     for (unsigned int i = 0; i != pl.size(); ++i) {
//         if (output.write(escaper.escape("    " + l.getSource(i) + "\n")))
//         return -1;
//     }
//
//     if (output.write(escaper.escape("\nConfiguration dictionary sources:\n")))
//         return -1;
//     const SourceList_t &p = configuration.getSources();
//     for (unsigned int i = 0; i != pl.size(); ++i) {
//         if (output.write(escaper.escape("    " + p.getSource(i) + "\n")))
//         return -1;
//     }
//
//     // configuration
//     std::ostringstream os;
//     os << configuration;
//     if (output.write(escaper.escape("\n" + os.str())))
//         return -1;
//
//     if (output.write(escaper.escape("\nApplication data:\n")))
//         return -1;
//     return dumpFragment(configuration, escaper, output, data);
// }

