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

    virtual ~RAM()
    {
        if (data)
            delete[] data;
 
    }

    T        *data;

    virtual void add_memory([[maybe_unused]]Memory<T> *mem) {}


    virtual size_t GetSize()
    {
        return this->size;
    }

//    virtual size_t GetBase()
//    {
//        return this->base;
//    }
//
//    virtual void SetBase(size_t base)
//    {
//        this->base = base;
//    }

    /**
     * @brief Return the value of the register.
     * @return T
     */
    virtual bool read(T &val, size_t index)
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

    virtual bool write(T val, size_t index)
    {
        if (index >= this->size)
            return false;
        data[index] = val;
        return true;
    };
};

//namespace emulator
//{
//
//template <mem_width W>
//class RAM : public Memory
//{
//public:
//    RAM(size_t size, size_t base) : 
//        Memory(size), base(base)
//    {
//        this->size = size;
//        if constexpr (W == U8)
//            data8  = new uint8_t[size];
//        else if constexpr (W == U16)
//            data16 = new uint16_t[size]; 
//        else if constexpr (W == U32)
//            data32 = new uint32_t[size];
//        else 
//            data64 = new uint64_t[size];
//        }
//    }
//
//    virtual ~RAM()
//    {
//        if (data8)
//            delete[] data8;
//        if (data16)
//            delete[] data16;
//        if (data32)
//            delete[] data32;
//        if (data64)
//            delete[] data64;
//    }
//
//    size_t    base;
//    uint8_t  *data8;
//    uint16_t *data16;
//    uint32_t *data32;
//    uint64_t *data64;
//
//    virtual void add_memory(Memory *mem) {}
//
//
//    virtual size_t GetSize()
//    {
//        return this->size;
//    }
//
//    virtual size_t GetBase()
//    {
//        return base;
//    }
//
//    virtual void SetBase(size_t base)
//    {
//        this->base = base;
//    }
//
//    /**
//     * @brief Return the value of the register.
//     * @return T
//     */
//    virtual bool read(uint64_t &val, size_t index)
//    {
//        if (index >= this->size) {
//            val = 0;
//            return false;
//        }
//        if constexpr (W == U8)
//            val = data8[index];
//        else if constexpr (W == U16)
//            val = data16[index];
//        else if constexpr (W == U32)
//            val = data32[index];
//        else 
//            val = data64[index];
//        return true;
//    };
//
//    /**
//     * @brief Set the value of a register to the argument.
//     * @param val
//     */
//
//    virtual bool write(uint64_t val, size_t index)
//    {
//        if (index >= this->size)
//            return false;
//        if constexpr (W == U8)
//            data8[index] = val;
//        else if constexpr (W == U16)
//            data16[index] = val;
//        else if constexpr (W == U32)
//            data32[index] = val;
//        else 
//            data64[index] = val;
//        return true;
//    };
//};
}
