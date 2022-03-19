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
 
#include <stdio.h>
#include <string>
#include <sstream>
#include "Command.h"

namespace core {

    class CmdFunc {
        std::string      name_;
        std::string      help_;
        class Object;
        Object& object_;
        typedef int (Object::*Parser)(std::stringstream& buffer) const;
        Parser   parser_;
        typedef int (Object::*Complete)(std::stringstream& buffer) const;
        Complete comp_;
        
        int nullcmd([[maybe_unused]]std::stringstream& buffer) { return 0; }
    public:
        template <typename T>
        CmdFunc(const std::string& n, const std::string& h, T& t, 
                int (T::*p)(std::stringstream&),
                int (T::*c)(std::stringstream&)) : name_(n), help_(h),
            object_(reinterpret_cast<Object &>(t)),
            parser_(reinterpret_cast<Parser>(p)),
            comp_(reinterpret_cast<Complete>(c)) 
            {}
            
        CmdFunc(const std::string& n, const std::string& h) :
            name_(n), help_(h),
            object_(reinterpret_cast<Object& >(*this)),
            parser_(reinterpret_cast<Parser>(&CmdFunc::nullcmd)),
            comp_(reinterpret_cast<Complete>(&CmdFunc::nullcmd))
            {}
            
        int parse(std::stringstream& buffer) const
        { 
            return (object_.*parser_)(buffer);
        }
        
        int complete(std::stringstream& buffer) const
        {
            return (object_.*comp_)(buffer);
        }
    };
    
    
    bool Command::do_cmd(const std::string& buffer)
    {
        std::stringstream sbuffer;
        sbuffer<<buffer;
        std::string cmd;
        
        sbuffer>>cmd;
        
        std::cerr << "Command: " << cmd << std::endl;
        return false;
    }
}
