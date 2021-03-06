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

#include "config.h"
#include <vector>
#include <variant>
#include <string>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "SimError.h"
#include "ConfigOption.h"

namespace emulator
{

    using Access_error = core::SimError<4>;

/**
 * @class Memory
 * @author rich
 * @date 25/05/21
 * @file Memory.h
 * @brief Memory class is the basic interface to memory type objects. The class will
 *     will return error on any access to memory.
 */
template <typename T>
class Memory
{
public:
    /**
     * Creates a new memory object of a given size.
     * @brief default constructer.
     * @param size
     * @return
     */
    Memory(const size_t size, const size_t base)
    {
        tot_size_ = size;
        size_ = size;
        base_ = base;
    }

    virtual ~Memory()
    {
    }

    /**
     * @brief Sets the name of the object for Configuration and loging purposes.
     * @param name New name to set.
     * @return
     */
    Memory& setName(const std::string& name)
    {
        name_ = name;
        return *this;
    }

    /**
     * @brief Returns the name of the object.
     * @return
     */
    const std::string& getName() const
    {
        return name_;
    }

    /**
      * @brief Returns size of memory in T units.
      * @return
      */
    virtual size_t getSize() const
    {
        return tot_size_;
    }

    /**
      * @brief Returns base index of memory in T units.
      * @return
      */
    virtual size_t getBase() const
    {
        return base_;
    }

    /**
     * @brief Sets the base index for this memory object in T units.
     * @param base - base index.
     */
    virtual void setBase(size_t base)
    {
        base_ = base;
    }

    /**
     * @brief Adds a memory module to be used under this one.
     *        This is used for memory interfaces the divide memory
     *        into regions.
     * @param mem - Memory object to attach.
     */
    virtual void addMemory([[maybe_unused]]std::shared_ptr<Memory> mem) {};

    /**
     * @brief Adds options to this CPU module.
     * @return Option parser.
     */
    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("Memory options");
        auto base_opt = option.add<core::ConfigValue<size_t>>("base", 
                    "Base location of memory", 0, &base_);
        return option;
    }
    
    /**
     * @brief Retrieve a value from memory or throw exception if no location.
     * @param val returned value.
     * @param index location to access.
     */
    virtual void Get([[maybe_unused]]T &val, [[maybe_unused]]size_t index)
    {
        throw Access_error{"Invalid memory location"};
    }

    /**
     * @brief Set memory to a value or throw exception if no location.
     * @param val returned value.
     * @param index location to access.
     */
    virtual void Set([[maybe_unused]]T val, [[maybe_unused]]size_t index)
    {
        throw Access_error{"Invalid memory location"};
    }
    
    /**
     * @brief Return the value of the memory at index
     * @param val - reference to result of memory access.
     * @param index - location to retrive.
     * @return true if access within this module, false otherwise.
     */
    virtual bool read(T &val, [[maybe_unused]]size_t index)
    {
        val = 0;
        return false;
    }

    /**
     * @brief Sets the value of the memory at index
     * @param val - Value to set.
     * @param index - location to retrive.
     * @return true if access within this module, false otherwise.
     */
    virtual bool write([[maybe_unused]]T val, [[maybe_unused]]size_t index)
    {
        return false;
    }

    /**
     * @brief Total amount of memory in system.
     */
    size_t tot_size_;

    /**
     * @brief Size of this memory module.
     */
    size_t size_;

    /**
     * @brief First address of memory module.
     */
    size_t base_;

    /**
     * @brief Name of this memory module.
     */
    std::string name_;
};

/**
 * @class MemEmpty
 * @author rich
 * @date 25/05/21
 * @file Memory.h
 * @brief Memory empty is a class the always returns no access.
 */
template <typename T>
class MemEmpty : public Memory<T>
{
public:
    /**
     * Creates a new empty memory object of a given size.
     * @brief default constructer.
     * @param size
     * @return
     */
    MemEmpty(const size_t size, const size_t base) : Memory<T>(size, base)
    {
    }
    virtual ~MemEmpty()
    {
    }
};

/**
 * @class MemFixed
 * @author rich
 * @date 25/05/21
 * @file Memory.h
 * @brief A memory controller that points to a single chuck of memory.
 */
template <typename T>
class MemFixed : public Memory<T>
{
public:
    /**
     * Creates a new memory controller, size will be replaced with
     *     the size of whatever memory module is attached.
     * @brief default constructer.
     * @param size
     * @return
     */
    MemFixed(const size_t size, const size_t base) : Memory<T>(size, base)
    {
    }
    virtual ~MemFixed()
    {
    }

    /**
     * @brief Returns size of memory in T units.
     * @return
     */
    virtual size_t getSize() const override
    {
        return this->size_;
    }

    /**
     * @brief Sets the memory that this controller will access.
     * @param mem New memory module.
     */
    virtual void addMemory(std::shared_ptr<Memory<T>> mem) override
    {
        mem_ = mem;
        rmem_ = mem_.get();
        // Update base and size from module.
        this->size_ = mem_->getSize();
        this->base_ = mem_->getBase();
    }

    /**
     * @brief Retrieve a value from memory or throw exception if no location.
     * @param val returned value.
     * @param index location to access.
     */
    virtual void Get(T &val, size_t index) override
    {
        if (index >= this->base_ && mem_ != nullptr)
            mem_->Get(val, index - this->base_);
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
        if (index >= this->base_ && mem_ != nullptr)
            mem_->Set(val, index - this->base_);
        else
            throw Access_error{"Invalid memory location"};
    }
    
    /**
     * @brief Read the memory at location index. Returns false if access
     *      outsize or range, or if no memory object.
     * @param val - Reference to value read.
     * @param index - Location in object to read.
     * @return true if access succeeded, false if out of range.
     */
    virtual
    bool read(T& val, size_t index) override
    {
        if (index >= this->base_)
            return rmem_->read(val, index - this->base_);
        val = 0;
        return false;
    };

    /**
    * @brief Write the memory at location index. Returns false if access
    *      outsize or range, or if no memory object.
    * @param val - Value to set memory too.
    * @param index - Location in object to read.
    * @return true if access succeeded, false if out of range.
    */
    virtual
    bool write(T val, size_t index) override
    {
        if (index < this->base_)
            return false;
        return rmem_->write(val, index - this->base_);
    };

    /**
     *  Pointer to memory device that holds actual values.
     */
    std::shared_ptr<Memory<T>> mem_{};
    
    /**
     *  Hold raw pointer for accesing speed.
     */
    Memory<T>                  *rmem_;
};


/**
 * @class MemArray
 * @author rich
 * @date 25/05/21
 * @file Memory.h
 * @brief Memory array is a array of memory object. An array of pointers to memory
 *     is created to hold from memory of size with chunk_size granulatiry of access.
 */
template <typename T>
class MemArray : public Memory<T>
{

public:
    /**
     * @brief Default constructor.
     * @param size - Size of memory region controlled.
     * @param chunk_size - Access granularity, must be power of 2.
     * @return
     */
    MemArray(const size_t size, const size_t chunk_size = 4096)
        : Memory<T>(size, 0)
    {
        empty_ = std::make_shared<MemEmpty<T>>(size, 0);
        this->size_ = size;
        // Make sure power of two.
        if ((chunk_size & (chunk_size - 1)) != 0)
            throw;
        // Figure out how many chucks we need.
        size_t num = size / chunk_size;
        // Compute index shift.
        for(shift_ = 0; chunk_size != (1llu << shift_); shift_++);
        // Allocate and initialize the memory.
        mem_ = new std::shared_ptr<Memory<T>> [num];
        // Set all entries to non-existant memory.
        for (size_t i = 0; i < num; i++) {
            mem_[i] = empty_;
        }
    }

    virtual ~MemArray()
    {
        delete[] mem_;
    }

    /**
     * @brief Returns size of memory in T units.
     * @return
     */
    virtual size_t getSize() const override
    {
        return this->size_;
    }

    /**
     * @brief Returns base address of memory.
     *         Note base is always 0 for Array memory.
     * @return
     */
    virtual size_t getBase() const override
    {
        return 0;
    }

    /**
     * @brief Sets the base, no - op for MemArray.
     * @param base
     */
    virtual void setBase([[maybe_unused]]size_t base) override {  }

    /**
     * @brief Add a region of memory to the array.
     *        Pointers to subregions are duplicated.
     * @param mem - Memory to add.
     */
    virtual void addMemory(std::shared_ptr<Memory<T>> mem) override
    {
        size_t base_address = mem->getBase() >> shift_;
        size_t top_address = (mem->getSize() >> shift_) + base_address;
        for (size_t i = base_address; i < top_address; i++) {
            mem_[i] = mem;
        }
    }
    
    /**
     * @brief Retrieve a value from memory or throw exception if no location.
     * @param val returned value.
     * @param index location to access.
     */
    virtual void Get(T &val, size_t index) override
    {
         // Compute bin address is located in.
        size_t base = index >> shift_;

        // Make sure in range and access it.
        if (index < this->size_) 
            mem_[base]->Get(val, index - mem_[base]->getBase());
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
       size_t base = index >> shift_;
        if (index < this->size_)
            return mem_[base]->Set(val, index - mem_[base]->getBase());
        else
            throw Access_error{"Invalid memory location"};
    }

    /**
    * @brief Read the memory at location index. Returns false if access
    *      outsize or range, or if no memory object.
    * @param val - Reference to value read.
    * @param index - Location in object to read.
    * @return true if access succeeded, false if out of range.
    */
    virtual
    bool read(T& val, size_t index) override
    {
        // Compute bin address is located in.
        size_t base = index >> shift_;

        // Make sure in range and access it.
        if (index < this->size_) 
            return mem_[base]->read(val, index - mem_[base]->getBase());
        val = 0;
        return false;
    };

    /**
    * @brief Write the memory at location index. Returns false if access
    *      outsize or range, or if no memory object.
    * @param val - Value to set memory too.
    * @param index - Location in object to read.
    * @return true if access succeeded, false if out of range.
    */
    virtual
    bool write(T val, size_t index) override
    {
        size_t base = index >> shift_;
        if (index < this->size_)
            return mem_[base]->write(val, index - mem_[base]->getBase());
        return false;
    };

    /**
     * @brief shift factor for determining bin.
     */
    size_t      shift_;

    /**
     * @brief Array of memory pointers.
     */

    std::shared_ptr<Memory<T>> empty_;
    std::shared_ptr<Memory<T>> *mem_;
};
}

#define REGISTER_MEM(systype, model, width) \
    namespace core { \
    class systype##_##model##_##MemFactory : public MemFactory { \
    public: \
        systype##_##model##_##MemFactory() \
        { \
            std::cout << "Registering Mem: " #model << "\n"; \
            systype::registerMem(#model, this); \
        } \
        virtual MEM_v create(const size_t size, const size_t base) { \
            return std::make_shared<emulator::model<width>>(size, base); \
        } \
    }; \
    static systype##_##model##_##MemFactory global_##systype##_##model##_##MemFactory; \
    };

#define REGISTER_SYSTEM_TEMPLATE_MEM \
    public: \
    static void registerMem(const string & name, core::MemFactory *factory) \
    { mem_factories.insert(make_pair(name, factory)); } \
    MEM_v create_mem(const string &model, const size_t size, const size_t base) { \
        if (mem_factories.count(model) == 0) \
            throw core::SystemError{"Unknown mem type: " + model}; \
        return mem_factories[model]->create(size, base); \
    } \
    private: \
    static map<string, core::MemFactory *> mem_factories; \
    public:

namespace core
{

using MEM_v = std::variant<std::shared_ptr<emulator::Memory<uint8_t>>,
      std::shared_ptr<emulator::Memory<uint16_t>>,
      std::shared_ptr<emulator::Memory<uint32_t>>,
      std::shared_ptr<emulator::Memory<uint64_t>>>;

/**
 * @class MemFactory
 * @author rich
 * @date 25/05/21
 * @file Memory.h
 * @brief Factory for generating Memory objects.
 */
class MemFactory
{
public:
    virtual MEM_v create(const size_t size, const size_t base) = 0;
};

}
