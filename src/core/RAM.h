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

#include "Memory.h"

namespace emulator
{

template <typename T>
class RAM : public Memory<T>
{
public:
    RAM(size_t size, size_t base) : 
        Memory<T>(size)
    {
        this->size = size;
        this->base = base;
        data  = new T[size];
    }

    virtual ~RAM() override
    {
        if (data)
            delete[] data;
    }

    T        *data;

    virtual size_t GetSize() const override
    {
        return this->size;
    }

    /**
     * @brief Return the value of the register.
     * @return T
     */
    virtual bool read(T &val, size_t index) override
    {
        if (index >= this->size) {
            val = 0;
            return false;
        }
        val = data[index];
        return true;
    };

    /**
     * @brief Set the value of a register to the argument.
     * @param val
     */

    virtual bool write(T val, size_t index) override
    {
        if (index >= this->size)
            return false;
        data[index] = val;
        return true;
    };
};

}
