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

/**
 * @class Device
 * @author rich
 * @date 21/03/21
 * @file Device.h
 * @brief
 */
template <typename T>
class Device
{
public:
    Device()
    {
    }
    
    Device(const std::string& name) : name_(name) {
    }

    virtual ~Device()
    {
    }

    virtual auto getType() const -> std::string
    {
        return "Device";
    }

    void showModel()
    {
        std::cout << "Device model = " << this->getType() << std::endl;
    }

    std::string name_;

    Device& SetName(const std::string& name)
    {
        name_ = name;
        return *this;
    }

    const std::string& GetName() const
    {
        return name_;
    }

    virtual void setAddress(size_t addr)
    {
	    addr_ = addr;
    }

    virtual size_t getAddress() const
    {
	    return addr_;
    }

    virtual size_t getSize() const
    {
	    return 1;
    }
    
    virtual void init() {}
    virtual void shutdown() {}
    virtual void start() {}
    virtual void reset() {}
    virtual void stop() {}
    virtual void step() {}
    virtual void run() {}
    virtual void examine() {}
    virtual void deposit() {}

    virtual bool input(T &val, [[maybe_unused]]size_t port)
    {
        val = 0;
        return false;
    }

    virtual bool output([[maybe_unused]]T val, [[maybe_unused]]size_t port)
    {
        return false;
    }
    
    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("Device Options");
        return option;
    }

protected:
    size_t addr_;
private:
};
}

#define REGISTER_SYSTEM_TEMPLATE_DEV \
    public: \
    static void registerDevice(const std::string & name, core::DeviceFactory *factory) \
    { dev_factories.insert(make_pair(name, factory)); } \
    DEV_v create_dev(const std::string &model) \
    { \
        if (dev_factories.count(model) == 0) \
            throw core::SystemError{"Unknown device type: " + model}; \
        return dev_factories[model]->create(model); \
    } \
    private: \
    static map<string, core::DeviceFactory *> dev_factories; \
    public:

#define REGISTER_DEVICE(systype, type) \
    namespace core { \
    class systype##DeviceFactory : public DeviceFactory { \
    public: \
        systype##DeviceFactory() \
        { \
            std::cout << "Registering Device: " #type << "\n"; \
            systype::registerDevice(#type, this); \
        } \
        virtual DEV_v create(const std::string & name) { \
            return std::make_shared<emulator::systype##_##type>(name);  \
        } \
    }; \
    static systype##DeviceFactory global_##systype##_##type##DeviceFactory; \
    };
    

namespace core {

using DEV_v = std::variant<std::shared_ptr<emulator::Device<uint8_t>>,
      std::shared_ptr<emulator::Device<uint16_t>>,
      std::shared_ptr<emulator::Device<uint32_t>>,
      std::shared_ptr<emulator::Device<uint64_t>>>;

class DeviceFactory
{
public:
    virtual DEV_v create(const std::string & name) = 0;
};

}
