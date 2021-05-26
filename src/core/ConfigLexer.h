/*
 * Author:      Richard Cornwell (rich@sky-visions.com)
 *
 * Copyright (C) 2021 Richard Cornwell.
 *
 * This file may be distributed under the terms of the Q Public License
 * as defined by Trolltech AS of Norway and appearing in the file
 * LICENSE.QPL included in the packaging of this file.
 *
 * THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING
 * THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#pragma once

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include "SimError.h"

namespace core {
    
using Lexical_error = SimError<1>;
using Syntax_error = SimError<2>;
using Config_error = SimError<3>;

/**
 * @brief List of valid tokens.
 */
enum class ConfigToken {
    Id, Str, Sys, Cpu, Mem, Dev, Unit, Ctl, Units, Number, Rparn, Lparn,
    Colon, Equal, Comma, Load, Mount, RO, EOFSym, Error
};

/**
 * @class ConfigLexer
 * @author rich
 * @date 25/05/21
 * @file ConfigLexer.h
 * @brief Scan the input string and return tokens for each term.
 */
class ConfigLexer
{
public:
    explicit ConfigLexer(std::istream &is);
    explicit ConfigLexer(std::istream *ps);

    ConfigLexer(const ConfigLexer&) = delete;
    ConfigLexer& operator=(const ConfigLexer&) = delete;

    ConfigLexer(ConfigLexer&&) = delete;
    ConfigLexer& operator=(ConfigLexer&&) = delete;

    ~ConfigLexer()
    {
        if (owns_input) delete p_input;
    }

    /**
     * @brief Return the current token value.
     * @return ConfigToken - token from last scan.
     */
    ConfigToken token() const
    {
        return cur_token;
    }

    /**
     * @brief Return the text of the last token found.
     * @return 
     */
    std::string token_text() const
    {
        return cur_token_text;
    }

    /**
     * @brief Return the numeric value of the last number scanned.
     * @return 
     */
    uint64_t token_value() const
    {
        return cur_token_value;
    }

    /**
     * @brief Advance to the next token.
     * @param keyword If set to true, match keyword values. 
     *                If set to false, just return identifiers.
     */
    void advance(bool keyword = true);

private:
    std::istream * p_input;
    bool owns_input;

    ConfigToken cur_token;
    std::string cur_token_text;
    uint64_t  cur_token_value;

    void init();

    
    /**
     * @brief Get next token.
     * @param keyword flag to indicate whether to scan for keywords.
     */
    ConfigToken get_token(bool keyword);
    std::string buffer;
    uint64_t    value;
};

}
