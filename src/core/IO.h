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
#include <variant>
#include <string>
#include <map>
#include "ConfigOption.h"

namespace emulator
{

template<typename T>
class CPU;

template<typename T>
class Memory;
};

#include "CPU.h"
#include "Memory.h"
#include "Device.h"

namespace emulator
{

/**
 * @class IO
 * @author rich
 * @date 21/03/21
 * @file IO.h
 * @brief
 */


template<typename T>
class IO
{
public:
    IO()
    {
    }

    virtual ~IO()
    {
    }

    virtual auto getType() const -> std::string
    {
        return "IO";
    }

    void showModel()
    {
        std::cout << "IO model = " << this->getType() << std::endl;
    }

    std::string name_;

    IO& SetName(const std::string& name)
    {
        name_ = name;
        return *this;
    }

    const std::string& GetName() const
    {
        return name_;
    }


    virtual void add_io([[maybe_unused]]std::shared_ptr<IO> io) {};

    virtual void add_device([[maybe_unused]]std::shared_ptr<Device<T>> dev) {};

    virtual void set_cpu(CPU<T>* cpu_v)
    {
        cpu = cpu_v;
    }

    virtual void set_memory(std::shared_ptr<Memory<T>> mem_v)
    {
        mem = mem_v;
    }

    virtual void init() {};
    virtual void shutdown() {};
    virtual void start() {};
    virtual void reset() {};
    virtual void stop() {};
    virtual void step() {};
    virtual void run() {};

    virtual bool input(T &val, [[maybe_unused]]size_t port)
    {
        val = 0;
        std::cerr << "IO_input " << std::hex << port << std::endl;
        return false;
    }
    virtual bool output([[maybe_unused]]T val, [[maybe_unused]]size_t port)
    {
        std::cerr << "IO_output " << std::hex << port << std::endl;
        return false;
    }

    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("IO Options");
        return option;
    }

protected:
    CPU<T>* cpu;
    std::shared_ptr<Memory<T>> mem;

private:

};
}


#define REGISTER_IO(systype, model) \
    namespace core { \
    class model##IOFactory : public IOFactory { \
    public: \
        model##IOFactory() \
        { \
            std::cout << "Registering IO: " #model << "\n"; \
            systype::registerIO(#model, this); \
        } \
        virtual IO_v create() { \
            return make_shared<emulator::systype##_io<model>>(); \
        } \
    }; \
    static model##IOFactory global_##model##IOFactory; \
    };

#define REGISTER_SYSTEM_TEMPLATE_IO \
    public: \
    static void registerIO(const string & name, core::IOFactory *factory) \
    { io_factories.insert(make_pair(name, factory)); } \
    IO_v create_io(const string &model) \
    { \
        if (io_factories.count(model) == 0) \
            throw core::SystemError{"Unknown io type: " + model}; \
        return io_factories[model]->create(); \
    } \
    private: \
    static map<string, core::IOFactory *> io_factories; \
    public:

#define REGISTER_IO_FIXED(systype) \
    namespace core { \
    class systype##IOFactory : public IOFactory { \
    public: \
        systype##IOFactory() \
        { \
            std::cout << "Registering IO: " #systype << "\n"; \
            systype::registerIO(#systype, this); \
        } \
        virtual IO_v create() { \
            return std::make_shared<emulator::##systype##_io>(); \
        } \
    }; \
    static systype##IOFactory global_##systype##IOFactory; \
    };


namespace core
{

using IO_v = std::variant<std::shared_ptr<emulator::IO<uint8_t>>,
      std::shared_ptr<emulator::IO<uint16_t>>,
      std::shared_ptr<emulator::IO<uint32_t>>,
      std::shared_ptr<emulator::IO<uint64_t>>>;

class IOFactory
{
public:
    virtual IO_v create() = 0;
};
}

namespace emulator
{

template<typename T>
class IO_map : public IO<T>
{
public:
    IO_map(const size_t num_devices) : max_ports_(num_devices)
    {
        nuldev_ = std::make_shared<Device<T>>();
        devices_ = new std::shared_ptr<Device<T>> [max_ports_];
        for (size_t i = 0; i < num_devices; i++)
            devices_[i] = nuldev_;
    }

    virtual ~IO_map()
    {
        delete[] devices_;
    }

    virtual void add_device(std::shared_ptr<Device<T>> dev) override
    {
      //  std::cerr << "Adding device: " << dev->GetName() << " " << std::hex << dev->getAddress() << std::endl;
        // Later these will throw an exception.
        size_t first_port = dev->getAddress();
        if (first_port > max_ports_)
            return;
        size_t num_ports = dev->getSize();
        if ((first_port + num_ports) > max_ports_)
            return;
        for (size_t i = 0; i < num_ports; i++) {
            devices_[first_port + i] = dev;
        }
    };

    virtual void init() override
    {
        for(size_t i = 0; i < max_ports_; i++) {
            devices_[i]->init();
        };
    };

    virtual void start() override
    {
        for(size_t i = 0; i < max_ports_; i++) {
            devices_[i]->start();
        };
    };

    virtual void reset() override
    {
        for(size_t i = 0; i < max_ports_; i++) {
            devices_[i]->reset();
        };
    };
    virtual void stop() override
    {
        for(size_t i = 0; i < max_ports_; i++) {
            devices_[i]->stop();
        };
    };
    virtual void step() override
    {
        for(size_t i = 0; i < max_ports_; i++) {
            devices_[i]->step();
        };
    };
    virtual void run() override
    {
        for(size_t i = 0; i < max_ports_; i++) {
            devices_[i]->run();
        };
    };
    virtual void shutdown() override
    {
        for(size_t i = 0; i < max_ports_; i++) {
            devices_[i]->shutdown();
        };
    };

    virtual bool input(T &val, size_t port) override
    {
        if (port > max_ports_)
            return false;
       // std::cerr << "IOInput()" << std::hex << port << std::endl;
        return devices_[port]->input(val, port);
    }

    virtual bool output(T val, size_t port) override
    {
        if (port > max_ports_)
            return false;
       // std::cerr << "IOOutput()" << std::hex << port << std::endl;
        return devices_[port]->output(val, port);
    }

private:
    size_t max_ports_;
    std::shared_ptr<Device<T>> nuldev_;
    std::shared_ptr<Device<T>> *devices_;
};
}
