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

/*
 * Process configuration file.
 *
 * Syntax:
 *     System <name> {  definition }
 *
 *   [] indicates optional parameters
 *
 * definition:=
 *     CPU type[:name][(options)]
 *     Memory type[:name][(options)] size[,base[,mask]] [load=file]
 *
 *     Optional for some types of simulators:
 *     IO type[:name][(options)] address[,mask]
 *
 *     Device type[:name][=address[,size]][(options)] [units=#]
 *
 *     Unit type[:name][=unit#][(options)] [format=fmt] [ro] [mount=file]
 *
 *     Control type[:name] device#1[,device#2[,device#3]....]
 *
 *     Example:
 *       device rp10:dpa=0270 units=8
 *       device tm10b:mta units=4
 *       control df10c dpa,mta
 *
 *
 *
 *
 *     Example:
 *
 */
#include <string>
#include <memory>
#include "System.h"
#include "ConfigLexer.h"

namespace core {

//using Lexical_error = SimError<1>;
//using Syntax_error = SimError<2>;
//using Config_error = SimError<3>;

class Config
{
public:
    Config() : sys(nullptr), p_lexer(nullptr)
    {
    }
    
    ~Config()
    {
    }

    bool operator()(const std::string& s);

    std::shared_ptr<core::System> sys;
private:
    core::ConfigLexer*  p_lexer;
    bool parse_line();
    bool parse_system();
    bool parse_cpu();
    bool parse_memory();
};

}
