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
 * $Id: tengparsercontext.h,v 1.5 2006-10-18 08:31:09 vasek Exp $
 *
 * DESCRIPTION
 * Teng parser context.
 *
 * AUTHORS
 * Michal Bukovsky <michal.bukovsky@firma.seznam.cz
 *
 * HISTORY
 * 2018-12-04  (burlog)
 *             Created.
 */

#ifndef TENGOVERRIDDENBLOCKS_H
#define TENGOVERRIDDENBLOCKS_H

#include <map>
#include <vector>

// TODO(burlog): remove
#include <iostream>

#include "position.h"
#include "teng/stringview.h"

namespace Teng {
namespace Parser {

/** The overridden block.
 */
struct OverriddenBlock_t {
    /** Returns the filename (not a path) of this implementation.
     */
    std::string impl_file() const {return pos.basename();}

    Pos_t pos;            //!< position of the overridden block
    int64_t addr;         //!< address of the first instruction
    std::string raw_data; //!< source code range that implements the block
};

/** Set of override blocks.
 */
class OverriddenBlocks_t {
public:
    // types
    using Overrides_t = std::vector<OverriddenBlock_t>;
    using Registry_t = std::map<std::string, Overrides_t>;
    using Entry_t = Registry_t::value_type;

    /** Inserts new implementation at the end of the overridden block list.
     */
    Entry_t &reg_block(std::string &&name, OverriddenBlock_t &&block) {
        auto ival = blocks.lower_bound(name);
        if (ival == blocks.end())
            ival = blocks.emplace_hint(ival, std::move(name), Overrides_t());
        else if (blocks.key_comp()(name, ival->first)) // it's operator<()
            ival = blocks.emplace_hint(ival, std::move(name), Overrides_t());
        ival->second.push_back(std::move(block));
        return *ival;
    }

protected:
    std::map<std::string, Overrides_t> blocks; //!< the set implementation
};

/** Represent current Teng extends block.
 */
struct ExtendsBlock_t {
    struct OverrideBlock_t {
        struct range_t {const char *begin; const char *end;};
        Pos_t pos;                   //!< position of override block
        int64_t addr;                //!< address of first instruction
        std::string name;            //!< the name/id of the block
        std::vector<range_t> chunks; //!< chunks of blocks source code
    };

    /** Starts new overridden block of desired name. If is not the address of
     * first instruction known then use -1 instead of it.
     */
    template <typename Context_t>
    void open_override(Context_t *ctx, std::string &&name, int64_t addr) {
        // append the first chunk, the nullptr will be replaced by lambda
        // bellow or by close_override()
        override_blocks.push_back({
            ctx->pos(),
            addr,
            std::move(name),
            {{ctx->lex1().current(), nullptr}}
        });

        // the lambda will be executed when the level 1 lexer will be popped,
        // it replace nullptr of last data chunk with appropriate end pointer,
        // and optionally appends new chunk of data if there is level 1 lexer
        auto i = override_blocks.size() - 1;
        ctx->lex1_stack.add_action([this, ctx, i] (auto &&lexer) {
            if (is_override_block_open()) {
                override_blocks[i].chunks.back().end = lexer.current();
                if (!ctx->lex1_stack.empty()) {
                    auto *begin = ctx->lex1().current();
                    override_blocks[i].chunks.push_back({begin, nullptr});
                }
            }
        });
    }

    /** Returns overridden block that consists from address of first
     * instruction of this block and range of source code that implements block.
     */
    OverriddenBlock_t close_override(const char *raw_data_end) {
        // set end pointer of the block if it not set yet
        auto &top = override_blocks.back();
        if (top.chunks.back().end == nullptr)
            top.chunks.back().end = raw_data_end;

        // concat all chunks of the block's source code
        std::string range;
        for (auto chunk: top.chunks)
            range.append(chunk.begin, chunk.end);

        // create and return new block
        OverriddenBlock_t block = {top.pos, top.addr, std::move(range)};
        override_blocks.pop_back();
        return block;
    }

    /** Returns the current override block that is on top of the stack.
     */
    OverrideBlock_t &override_block() {return override_blocks.back();}

    /** Returns true if override block is open.
     */
    bool is_override_block_open() const {return !override_blocks.empty();}

    // the blocks can be nested - we need the stack
    using Blocks_t = std::vector<OverrideBlock_t>;

    Pos_t pos;                 //!< positon of the extends block token
    int64_t nesting_level = 0; //!< how many nested extends blocks are
    int64_t super_addr = -1;   //!< address of the super implementation
    std::string base_file;     //!< the filename from which this inherits
    Blocks_t override_blocks;  //!< set of overridden blocks
};

} // namespace Parser
} // namespace Teng

#endif /* TENGOVERRIDDENBLOCKS_H */

