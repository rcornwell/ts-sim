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
#include <variant>
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
 * @brief The CPU class represents individual compute units. There can be multiple types
 * of these. Subclassing should template with a CPU type template.
 */


template <typename T>
class CPU
{
public:

    /**
     * @brief Default constructer.
     */
    CPU()
    {
        io = nullptr;
        running = false;
        pc = 0;
    };

    virtual ~CPU()
    {
    };

    /**
     * @brief CPU object can't be copied.
     */
    CPU(const CPU&) = delete;

    /**
     * @brief Return the type of this object, subclass should override to
     * return type of CPU.
     * @return CPU model string.
     */
    virtual std::string getType() const
    {
        return "CPU";
    };

    /**
     * @brief Show type of CPU.
     */
    void showModel()
    {
        std::cout << "CPU model = " << this->getType() << std::endl;
    };

    /**
     * @brief Set the name of the CPU.
     * @param name - new name to assign to CPU.
     * @return CPU object.
     */
    CPU& setName(const std::string& name)
    {
        name_ = name;
        return *this;
    };

    /**
     * @brief Returns the name of the CPU.
     * @return name string.
     */
    const std::string& getName() const
    {
        return name_;
    };

    /**
     * @brief Set the program counter.
     * @param pc_v New program counter value.
     * @return CPU object.
     */
    virtual
    CPU& setPC(const size_t pc_v)
    {
        pc = pc_v;
        return *this;
    };

    /**
     * @brief Return the current value of the program counter.
     * @return Program counter.
     */
    virtual
    size_t getPC() const
    {
        return pc;
    };

    /**
     * @brief Set the master memory controller.
     * @param mem_v Memory controller to use.
     * @return CPU object.
     */
    virtual
    CPU& setMem(std::shared_ptr<Memory<T>> mem_v)
    {
        sh_mem = mem_v;
        mem = sh_mem.get();
        return *this;
    };

    /**
     * @brief Return the current memory controller.
     */
    virtual
    std::shared_ptr<Memory<T>> getMem() const
    {
        return sh_mem;
    };

    /**
     * @brief Add either a new memory controller or a sub memory controller.
     * @param mem_v Controller to add.
     */
    virtual
    void addMemory(std::shared_ptr<Memory<T>> mem_v)
    {
        if (sh_mem == nullptr) {
            sh_mem = mem_v;
            mem = sh_mem.get();
        } else {
            sh_mem->addMemory(mem_v);
        }
    };

    /**
     * @brief Set defualt I/O controller.
     * @param io_v IO controller to set.
     * @return CPU object.
     */
    virtual
    CPU& setIO(std::shared_ptr<IO<T>> io_v)
    {
        std::cerr << "Setting IO(): " << io_v->getName() << std::endl;
        io = io_v;
        return *this;
    };

    /**
     * @brief Get the default I/O controller.
     * @return IO controller.
     */
    virtual
    core::IO_v getIO() const
    {
        std::cerr << "Getting IO(): " << io->getName() << std::endl;
        return io;
    };

    /**
     * @brief Add a sub I/O controller.
     * @param io_v Controller to add.
     */
    virtual
    void addIo(std::shared_ptr<IO<T>> io_v)
    {
        std::cerr << "Adding IO()" << std::endl;
        if (io == nullptr)
            io = io_v;
        else
            io->addIo(io_v);
    };

    /**
     * @brief Tell whether this type of CPU needs I/O controllers.
     * @return false if need I/O controllers. true if none needed.
     */
    virtual bool noIO() const
    {
        return false;
    };

    /**
     * @brief Called after the unit has been created, to do any final
     *   Initialization as requireed by the unit.
     */
    virtual void init()
    {
        std::cerr << "Init CPU()" << std::endl;
    };

    /**
     * @brief Called after the I/O and memory controllers have been attached
     *  but before the devices have been attached. This hooks the IO controller
     *  up to the CPU and Memory so it can interact with both.
     */
    virtual void init_io()
    {
        io->setCpu(this);
        io->setMemory(sh_mem);
    };

    /**
     * @brief Called after init but before actually actually running the simulation.
     * Any last minute initialization should be here.
     */
    virtual void start()
    {
        running = true;
        io->start();
    };

    /**
     * @brief Called to reset the CPU and potientialy any attached devices.
     */
    virtual void reset()
    {
        io->reset();
    };

    /**
     * @brief Excute one instruction.
     * @return Returns time in nanoseconds for operation.
     */
    virtual uint64_t step()
    {
        io->step();
        return 0;
    };

    /**
     * @brief Run the simulation until a stop condition is found.
     */
    virtual void run()
    {
        io->run();
        while(running) {
            (void)step();
        }
    };

    /**
     * @brief Called to stop any devices when simulation ends. Or is stopped by the user.
     */
    virtual void stop()
    {
        running = false;
        io->stop();
    };

    /**
     * @brief Called before simulation ends, to close out any devices.
     */
    virtual void shutdown()
    {
        io->shutdown();
    };

    virtual void trace() {};

    /**
     * @brief Return optional settings for this CPU module.
     * @return ConfigOptions object of supported options.
     */
    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("CPU Options");
        return option;
    };

//    virtual void examine(uint64_t& val, size_t addr)
//    {
//        mem->read(val, addr);
//    };
//
//    virtual void deposit(MT val, size_t addr)
//    {
//        mem->write(val, addr);
//    };
    /**
     * @brief Name of CPU object.
     */
    std::string name_;

    /**
     * @brief Is the CPU running or not.
     */
    bool running;

    /**
     * @brief Program Counter.
     */
    size_t pc;

    /**
     * @brief Pointer to shared pointer object passed to this object.
     */
    std::shared_ptr<Memory<T>>  sh_mem{};

    /**
     * @brief Raw pointer to memory object, used for speed.
     */
    Memory<T> *mem;

    /**
     * @brief Pointer to IO controller.
     */
    std::shared_ptr<IO<T>>  io{};

private:
};

}

/**
 * @brief Register a CPU model type.
 */
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

/**
 * @brief Placed in a CPU class to provide the default CPU constructors.
 */
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

/**
 * @class CPUFactory
 * @author rich
 * @date 04/06/21
 * @file CPU.h
 * @brief Factory for creating CPU type objects.
 */
class CPUFactory
{
public:
    virtual CPU_v create() = 0;
};

}
