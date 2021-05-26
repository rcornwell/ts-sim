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

namespace emulator
{

class i8080_io : public IO<mem_data>
{
public:
    i8080_io()
    {
    }
    virtual ~i8080_io()
    {
    }
    
    virtual void init() {};
    virtual void shutdown() {};
    virtual void start() {};
    virtual void reset() {};
    virtual void stop() {};
    virtual void step() {};
    virtual void run() {};

    virtual bool input(mem_data &val, size_t port)
    {
        val = 0;
        if (port != 0)
            return false;
        return true;
    }
    
    virtual bool output(mem_data val, size_t port)
    {
        uint8_t data;
        uint16_t addr;
        if (port == 1) {
            switch(cpu->regs[C]) {
            case 9:   // output
                addr = cpu->regpair<DE>();
                do {
                    mem->read(data, addr++);
                    // data = mem[addr++] & 0x7f;
                    if (data != '$')
                        std::cout << data;
                } while (data != '$');
                break;
            case 2: // output
                data = cpu->regs[E] & 0x7f;
                std::cout << data;
                break;
            }
            return true;
        }
        return false;
    }
};

}

