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

template<typename T>
class IO;
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
 * @brief Abstract class for representing I/O controllers.
 */
template<typename T>
class IO
{
public:
    /**
     * @brief Default constructor.
     * @return
     */
    IO()
    {
    }

    virtual ~IO()
    {
    }

    /**
     * @brief Returns the name of this type of I/O controller.
     * @return name string.
     */
    virtual std::string getType() const
    {
        return "IO";
    }

    /**
     * @brief Reports the name of this I/O controller.
     */
    void showModel()
    {
        std::cout << "IO model = " << this->getType() << std::endl;
    }

    /**
     * @brief Sets the name of this I/O controller.
     * @param name Name of controller.
     * @return self.
     */
    IO& setName(const std::string& name)
    {
        name_ = name;
        return *this;
    }

    /**
     * @brief Returns the name assigned to this I/O controller.
     * @return name string.
     */
    const std::string& getName() const
    {
        return name_;
    }

    /**
     * @brief Adds a sub I/O controller to this object.
     * @param io I/O controller to attach too.
     */
    virtual void addIo([[maybe_unused]]std::shared_ptr<IO> io) {};

    /**
     * @brief Adds a device to the I/O controller. The I/O controller will
     * query the device to find out where to place it.
     * @param dev device to attach.
     */
    virtual void addDevice([[maybe_unused]]std::shared_ptr<Device<T>> dev) {};

    /**
     * @brief Sets the CPU that this I/O controller is attached to.
     * @param cpu_v CPU object.
     */
    virtual void setCpu(CPU<T>* cpu_v)
    {
        cpu = cpu_v;
    }

    /**
     * @brief Sets the memory controller this object is attached to. Used for
     * direct memory transfers between a device and memory.
     * @param mem_v Memory controller.
     */
    virtual void setMemory(std::shared_ptr<Memory<T>> mem_v)
    {
        mem = mem_v;
    }

    /**
    * @brief Called after all I/O controllers and Devices have been added.
    * This should propogate init down to all attached devices and I/O controllers.
    */
    virtual void init()
    {
        std::cerr << "Init IO()" << std::endl;
    };

    /**
     * @brief Called after init but before actually actually running the simulation.
     * Any last minute initialization should be here.
     */
    virtual void start()
    {
    };

    /**
     * @brief Called to reset the the I/O devices. Should propogate to each device.
     */
    virtual void reset()
    {
    };

    /**
     * @brief Step one I/O cycle.
     */
    virtual void step()
    {
    };

    /**
     * @brief Called when run starts to initialize for continuous operation.
     */
    virtual void run()
    {
    };

    /**
     * @brief Called to stop any devices when simulation ends. Or is stopped by the user.
     */
    virtual void stop()
    {
    };

    /**
     * @brief Called before simulation ends, to close out any devices.
     */
    virtual void shutdown()
    {
    };

    /**
     * @brief Called to transfer data from I/O device to a CPU.
     * @param val Value to read.
     * @param port Address of device.
     * @return true if device exists, false if device does not exist.
     */
    virtual bool input(T &val, [[maybe_unused]]size_t port)
    {
        val = 0;
        std::cerr << "IO_input " << std::hex << port << std::endl;
        return false;
    };

    /**
     * @brief Called to transfer data from a CPU to a I/O device.
     * @param val Value to write.
     * @param port Address of device.
     * @return true if device exists, false if device does not exist.
     */
    virtual bool output([[maybe_unused]]T val, [[maybe_unused]]size_t port)
    {
        std::cerr << "IO_output " << std::hex << port << std::endl;
        return false;
    }

    /**
     * @brief Called to transfer status from I/O device to a CPU. Some simulators may not call this.
     * @param val Status read read.
     * @param port Address of device.
     * @return true if device exists, false if device does not exist.
     */
    virtual bool status(T &val, [[maybe_unused]]size_t port)
    {
        val = 0;
        std::cerr << "IO_input " << std::hex << port << std::endl;
        return false;
    };

    /**
     * @brief Called to transfer command from a CPU to a I/O device. Some simulators my not call this.
     * @param val Value to write.
     * @param port Address of device.
     * @return true if device exists, false if device does not exist.
     */
    virtual bool command([[maybe_unused]]T val, [[maybe_unused]]size_t port)
    {
        std::cerr << "IO_output " << std::hex << port << std::endl;
        return false;
    }
    /**
     * @brief Configutation options for this I/O controller.
     * @return set of options.
     */
    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("IO Options");
        return option;
    }

protected:
    /**
     * @brief Holds a pointer to the CPU who owns this I/O device.
     */
    CPU<T>* cpu;

    /**
     * @brief Pointer to memory controller who device can access.
     */
    std::shared_ptr<Memory<T>> mem;

    /**
     * @brief Name of controller.
     */
    std::string name_;

private:

};
}

/**
 * \def Used to define a IO device.
 * @brief Classes should be names of the format:
 *  systemtype_model.
 */
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

/**
 * @class IO_map
 * @author rich
 * @date 04/06/21
 * @file IO.h
 * @brief IO_map is a class that handles generic I/O addressing. Where there
 * is one controller which has a range of addresses.
 */
template<typename T>
class IO_map : public IO<T>
{
public:
    /**
     * @brief Default constructor.
     * @param num_devices
     * @details IO_map, mantains an array of ports at which various devices can be attached.
     * Default is to set them to the empty device which returns no access for all operations.
     */
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
    };

    /**
     * @brief Adds a device to an IO_map
     * @param dev - device to add.
     * @details Asks the device for it's address and how many ports it uses. Then points those ports to the
     * device. If the device gives an address that is out of range an exception will be thrown.
     */
    virtual void addDevice(std::shared_ptr<Device<T>> dev) override
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
        // Let device know who to talk to for Direct Memory Access.
        dev->setIO(this);
    };

    /**
    * @brief Called after all I/O controllers and Devices have been added.
    * This should propogate init down to all attached devices and I/O controllers.
    */
    virtual void init() override
    {
        for(size_t i = 0; i < max_ports_; i += devices_[i]->getSize() ) {
            devices_[i]->init();
        };
    };

    /**
     * @brief Called after init but before actually actually running the simulation.
     * Any last minute initialization should be here.
     */
    virtual void start() override
    {
        for(size_t i = 0; i < max_ports_; i += devices_[i]->getSize() ) {
            devices_[i]->start();
        };
    };

    /**
     * @brief Called to reset the the I/O devices. Should propogate to each device.
     */
    virtual void reset() override
    {
        for(size_t i = 0; i < max_ports_; i += devices_[i]->getSize() ) {
            devices_[i]->reset();
        };
    };

    /**
     * @brief Step one I/O cycle.
     */
    virtual void step() override
    {
        for(size_t i = 0; i < max_ports_; i += devices_[i]->getSize() ) {
            devices_[i]->step();
        };
    };

    /**
     * @brief Called when run starts to initialize for continuous operation.
     */
    virtual void run() override
    {
        for(size_t i = 0; i < max_ports_; i += devices_[i]->getSize() ) {
            devices_[i]->run();
        };
    };

    /**
     * @brief Called to stop any devices when simulation ends. Or is stopped by the user.
     */
    virtual void stop() override
    {
        for(size_t i = 0; i < max_ports_; i += devices_[i]->getSize() ) {
            devices_[i]->stop();
        };
    };

    /**
     * @brief Called before simulation ends, to close out any devices.
     */
    virtual void shutdown() override
    {
        for(size_t i = 0; i < max_ports_; i += devices_[i]->getSize() ) {
            devices_[i]->shutdown();
        };
    };

    /**
     * @brief Called to transfer data from I/O device to a CPU.
     * @param val Value to read.
     * @param port Address of device.
     * @return true if device exists, false if device does not exist.
     */
    virtual bool input(T &val, size_t port) override
    {
        if (port > max_ports_)
            return false;
        // std::cerr << "IOInput()" << std::hex << port << std::endl;
        return devices_[port]->input(val, port);
    };

    /**
      * @brief Called to transfer data from a CPU to a I/O device.
      * @param val Value to write.
      * @param port Address of device.
      * @return true if device exists, false if device does not exist.
      */
    virtual bool output(T val, size_t port) override
    {
        if (port > max_ports_)
            return false;
        // std::cerr << "IOOutput()" << std::hex << port << std::endl;
        return devices_[port]->output(val, port);
    };

    /**
     * @brief Called to transfer status from I/O device to a CPU.
     * @param val Status read read.
     * @param port Address of device.
     * @return true if device exists, false if device does not exist.
     */
    virtual bool status(T &val, size_t port) override
    {
        if (port > max_ports_)
            return false;
        // std::cerr << "IOInput()" << std::hex << port << std::endl;
        return devices_[port]->status(val, port);
    };

    /**
     * @brief Called to transfer command from a CPU to a I/O device.
     * @param val Value to write.
     * @param port Address of device.
     * @return true if device exists, false if device does not exist.
     */
    virtual bool command(T val, size_t port) override
    {
        if (port > max_ports_)
            return false;
        // std::cerr << "IOOutput()" << std::hex << port << std::endl;
        return devices_[port]->command(val, port);
    };

    private:
    /**
     * @brief Holds the maximum number of I/O ports that this I/O controller can handle.
     */
    size_t max_ports_;

    /**
      * @var
      * @brief Default device, should return non-accesable for all accesses attempted.
      */
    std::shared_ptr<Device<T>> nuldev_;

    /**
      * @var 
      * @brief Array of devices to access.
      */
    std::shared_ptr<Device<T>> *devices_;
};
}
