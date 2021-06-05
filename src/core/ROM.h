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

    
/**
 * @class ROM
 * @author rich
 * @date 01/06/21
 * @file ROM.h
 * @brief ROM is a generic read-only memory array. All writes are ignored.
 */
template <typename T>
class ROM : public Memory<T>
{
    public:
    /**
     * @brief Default constructor.
     * @param size size of memory to create.
     * @param base base address of memory. Used by super-classes to 
     *     locate the memory in the address space.
     */
    ROM(const size_t size, const size_t base) : 
        Memory<T>(size, base)
    {
        this->size_ = size;
        this->base_ = base;
        data_  = new T[size];
    }

    virtual ~ROM() override
    {
        if (data_)
            delete[] data_;
    }

    T        *data_;

    /**
     * @brief Return the size of this chunk of memory.
     * @return size of memory 
     */
    virtual size_t getSize() const override
    {
        return this->size_;
    }
    
    /**
     * @brief Retrieve a value from memory or throw exception if no location.
     * @param val returned value.
     * @param index location to access.
     */
    virtual void Get(T &val, size_t index) override
    {
        // Make sure in range and access it.
        if (index < this->size_) 
            val = data_[index];
        else
            throw Access_error{"Invalid memory location"};
    }

    /**
     * @brief Set memory to a value or throw exception if no location.
     * @param val returned value.
     * @param index location to access.
     */
    virtual void Set(T val, size_t index) override
    {
        if (index < this->size_)
            data_[index] = val;
        else
            throw Access_error{"Invalid memory location"};
    }

    /**
     * @brief Return the value of the register.
     * @return T
     */
    virtual bool read(T &val, size_t index) override
    {
        if (index >= this->size_) {
            val = 0;
            return false;
        }
        val = data_[index];        
        return true;
    };

    /**
     * @brief Do not allow value to be changed.
     * @param val
     */

    virtual bool write([[maybe_unused]]T val, size_t index) override
    { 
        if (index >= this->size_)
            return false;
        return true;
    };
};

}
