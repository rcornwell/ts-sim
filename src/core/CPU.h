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

#include <iostream>
#include <memory>
#include <string>
#include <map>
#include "Memory.h"
#include "IO.h"
#include "ConfigOption.h"

namespace emulator
{

enum mem_width {
    U8, U16, U32, U64
};

enum errors {
    NXM,
    ILL_INS,
};

/**
 * @class CPU
 * @author rich
 * @date 21/03/21
 * @file CPU.h
 * @brief
 */


template <typename T>
class CPU
{
public:

    CPU() 
    {
        io = nullptr;
        running = false;
        pc = 0;
    };

    virtual ~CPU() {
    };
    
    CPU(const CPU&) = delete;

    virtual auto getType() const -> std::string
    {
        return "CPU";
    }

    void showModel()
    {
        std::cout << "CPU model = " << this->getType() << std::endl;
    }
    
    std::string name_;
    
    CPU& SetName(const std::string& name)
    {
        name_ = name;
        return *this;
    }
    
    const std::string& GetName() const
    {
        return name_;
    }

    bool running;

    size_t pc;

    virtual
    CPU& SetPC(const size_t pc_v)
    {
        pc = pc_v;
        return *this;
    }

    virtual
    size_t GetPC() const
    {
        return pc;
    }

    std::shared_ptr<Memory<T>>  sh_mem{};
    Memory<T> *mem;
    
    virtual
    CPU& SetMem(std::shared_ptr<Memory<T>> mem_v)
    {
        sh_mem = mem_v;
        mem = sh_mem.get();
        return *this;
    }
    
    virtual
    std::shared_ptr<Memory<T>> GetMem() const
    {
        return sh_mem;
    }
    
    virtual
    void add_memory(std::shared_ptr<Memory<T>> mem_v)
    {
        if (sh_mem == nullptr) {
            sh_mem = mem_v;
            mem = sh_mem.get();
        } else {
            sh_mem->add_memory(mem_v);
        }
    }

    std::shared_ptr<IO<T>>  io{};
    
    virtual
    CPU& SetIO(std::shared_ptr<IO<T>> io_v)
    {
        io = io_v;
        return *this;
    }
    
    virtual
    std::shared_ptr<IO<T>> GetIO()
    {
        return io;
    }
    
    virtual
    void add_io(std::shared_ptr<IO<T>> io_v)
    {
        if (io == nullptr)
            io = io_v;
        else
            io->add_io(io_v);
    }

    virtual void trace() {};

    virtual void init() { std::cerr << "Init CPU()" << std::endl; };

    virtual void init_io() {
	    io->set_cpu(this);
	    io->set_memory(sh_mem);
    }

    virtual void shutdown() {};

    virtual void start()
    {
        running = true;
    };

    virtual void reset() {};

    virtual void stop()
    {
        running = false;
    };

    virtual uint64_t step()
    {
        return 0;
    };

    virtual void run() {};
    
    virtual
    core::ConfigOptionParser options() {
        core::ConfigOptionParser option("CPU Options");
        return option;
    }

//    virtual void examine(uint64_t& val, size_t addr)
//    {
//        mem->read(val, addr);
//    };
//
//    virtual void deposit(MT val, size_t addr)
//    {
//        mem->write(val, addr);
//    };

private:
};


}

#define REGISTER_CPU(systype, model) \
    namespace core { \
    class model##CPUFactory : public CPUFactory { \
    public: \
        model##CPUFactory() \
        { \
            std::cout << "Registering CPU: " #model << "\n"; \
            systype::registerCPU(#model, this); \
        } \
        virtual CPU_v create() { \
            return std::make_shared<emulator::systype##_cpu<emulator::model>>(); \
        } \
    }; \
    static model##CPUFactory global_##model##CPUFactory; \
    };

#define REGISTER_SYSTEM_TEMPLATE_CPU \
    public: \
    static void registerCPU(const string & name, core::CPUFactory *factory) \
    { cpu_factories.insert(make_pair(name, factory)); } \
    CPU_v create_cpu(const string &model) { \
     /*   if (number_cpus() >= max_cpus()) throw core::SystemError{"To many CPUs defined"};*/ \
        if (cpu_factories.count(model) == 0) \
            throw core::SystemError{"Unknown cpu type: " + model}; \
        return cpu_factories[model]->create(); \
    } \
    private: \
    static map<string, core::CPUFactory *> cpu_factories; \
    public:

namespace core
{

    
using CPU_v = std::variant<std::shared_ptr<emulator::CPU<uint8_t>>,
             std::shared_ptr<emulator::CPU<uint16_t>>, 
             std::shared_ptr<emulator::CPU<uint32_t>>,
             std::shared_ptr<emulator::CPU<uint64_t>>>;
             
class CPUFactory
{
public:
    virtual CPU_v create() = 0;
};

}

