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
#include <iostream>
#include <memory>
#include "System.h"

namespace core
{

using namespace std;

class i8080 : public System
{
public:
    i8080()
    {
    }
    virtual ~i8080()
    {
    }
    
    virtual auto getType() const -> string {
        return "i8080";
    }
    
    void showType() {
        cout << "Class Type = " << this->getType() << endl;
    }
    
    virtual size_t max_cpus() { return 1; }
    
//    virtual void add_cpu(core::CPU_v cpu) {
 //       this->cpu = std::get<shared_ptr<emulator::CPU<uint8_t>>>(cpu);
  //  }
       

REGISTER_SYSTEM_TEMPLATE_CPU
REGISTER_SYSTEM_TEMPLATE_MEM
REGISTER_SYSTEM_TEMPLATE_IO
REGISTER_SYSTEM_TEMPLATE_DEV

//    
//    virtual void init();
//    virtual void shutdown();
//    virtual void run();
//    virtual void stop();

};
}


