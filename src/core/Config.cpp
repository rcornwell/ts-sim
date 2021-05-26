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


#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include "SimError.h"
#include "ConfigLexer.h"
#include "ConfigOption.h"
#include "Config.h"

namespace core
{

using namespace std;

bool Config::operator()(const string& s)
{
    istringstream ist{s};
    p_lexer = new ConfigLexer{ist};
    bool r = parse_line();
    delete p_lexer;
    return r;
}

bool Config::parse_line()
{
    try {
        p_lexer->advance();
    } catch (const Lexical_error& e) {
        cout << "Error: " << e.get_message() << endl;
        return false;
    };
    ConfigToken key = p_lexer->token();

    try {
        do {
            switch (key) {
            // System <name>
            case ConfigToken::Sys:
                if (!parse_system())
                    return false;
                break;
            //     CPU <type>[:<name>][(<options>)]
            case ConfigToken::Cpu:
                if (!parse_cpu())
                    return false;
                break;
            //     Memory <type>[:name][=<cpu_name>,  or *][(<options>)] [load=<path>]
            case ConfigToken::Mem:
                if (!parse_memory())
                    return false;
                break;
            case ConfigToken::EOFSym:
                return true;

            default:
                cout << "Configuration file error: unknown key: " << endl;
                return false;
            }

            p_lexer->advance();
            key = p_lexer->token();
        } while (key != ConfigToken::EOFSym);
    } catch(Lexical_error& e) {
        cout << "Lexical_error: " << e.get_message() << endl;
        return false;
    } catch(Config_error& e) {
        cout << "Config error: " << e.get_message() << endl;
        return false;
    }
    return true;
    //        Id, Str, Sys, Cpu, Dev, Unit, Ctl, Units, Number, Rparn, Lparn,
    //Colon, Equal, Comma, Load, Mount, RO, EOFSym, Error
}


bool Config::parse_system()
{
    // Make sure that system is defined only once.
    if (sys != nullptr) {
        cout << "System can only be used once." << endl;
        return false;
    }
    // Must be followed by a name.
    p_lexer->advance();
    if (p_lexer->token() != ConfigToken::Id) {
        throw Config_error{"System must be followed by a name"};
    }

    // See if we can create one.
    sys = core::System::create(p_lexer->token_text());

    return true;
}

bool Config::parse_cpu()
{
    CPU_v  cpu;

    // System must be first.
    if (sys == nullptr) {
        cout << "System must be defined first." << endl;
        return false;
    }

    // Check if not too many CPU's
    if (sys->number_cpus() >= sys->max_cpus()) {
        cout << "Too many cpu's defined." << endl;
        return false;
    }

    // Look for type
    try {
        p_lexer->advance(false);
        if (p_lexer->token() != ConfigToken::Id) {
            throw Config_error{"CPU must be followed by model."};
        }
    } catch (const Lexical_error& e) {
        cout << e.get_message() << endl;
        return false;
    }


    // Try to create one.
    try {
        cpu = sys->create_cpu(p_lexer->token_text());
    } catch (SystemError& e) {
        cout << e.get_message() << endl;
        return false;
    }

    // See if optional name.
    try {
        p_lexer->advance();
        // Check if it is a colon.
        if (p_lexer->token() == ConfigToken::Colon) {
            p_lexer->advance(false);
            // Should be an identifier.
            if (p_lexer->token() == ConfigToken::Id) {
                auto caller = [name = p_lexer->token_text()](const auto& obj) {
                    obj->SetName(name);
                };
                std::visit(caller, cpu);
            }
            p_lexer->advance();
        }

        // Check for an option.
        if (p_lexer->token() == ConfigToken::Rparn) {
            auto caller = [](const auto& obj) {
                return obj->options();
            };
            ConfigOptionParser options = std::visit(caller, cpu);
            options.parse(p_lexer);
        }
    } catch (const Lexical_error& e) {
        cout << e.get_message() << endl;
        return false;
    }
    sys->add_cpu(cpu);
    return true;
}

//     Memory <type>[:name][=<cpu_name>,  or *] size [(<options>)] [load=<path>]
bool Config::parse_memory()
{
    MemInfo  meminfo;
    MEM_v    mem;
    size_t   size;
    string   name;

    // System must be first.
    if (sys == nullptr) {
        cout << "System must be defined first." << endl;
        return false;
    }

    // Look for type
    try {
        p_lexer->advance(false);
        if (p_lexer->token() != ConfigToken::Id) {
            throw Config_error{"Memory must be followed by model."};
        }
    } catch (const Lexical_error& e) {
        cout << e.get_message() << endl;
        return false;
    };

    // Try to create one.
    try {
        mem = sys->create_mem(p_lexer->token_text(), size);
    } catch (SystemError& e) {
        cout << e.get_message() << endl;
        return false;
    }

    meminfo.mem = mem;

    // See if optional name.
    try {
        p_lexer->advance();
        // Check if it is a colon.
        if (p_lexer->token() == ConfigToken::Colon) {
            p_lexer->advance(false);
            // Should be an identifier.
            if (p_lexer->token() == ConfigToken::Id) {
                auto caller = [name = p_lexer->token_text()](const auto& obj) {
                    obj->SetName(name);
                };
                std::visit(caller, mem);
            }
            p_lexer->advance();
        }

        // Check for an option.
        if (p_lexer->token() == ConfigToken::Rparn) {
            auto caller = [](const auto& obj) {
                return obj->options();
            };
            ConfigOptionParser options = std::visit(caller, mem);
            options.parse(p_lexer);
        }
    } catch (const Lexical_error& e) {
        cout << e.get_message() << endl;
        return false;
    }
    sys->add_memory(meminfo);
    return true;
}

}
