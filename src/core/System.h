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
#include <string>
#include <map>
#include <iostream>
#include <memory>
#include <vector>
#include <variant>
#include "SimError.h"
#include "CPU.h"
#include "IO.h"
#include "Memory.h"



#define REGISTER_SYSTEM(systype) \
    namespace core { \
    class systype##Factory : public SystemFactory { \
    public: \
        systype##Factory() \
        { \
            std::cout << "Registering: " #systype << "\n"; \
            System::registerType(#systype, this); \
            std::cout << "Registered" << "\n"; \
        } \
        virtual std::shared_ptr<System>create() { \
            return std::make_shared<systype>(); \
        } \
    }; \
    static systype##Factory global_##systype##Factory; \
    };
    

namespace core
{
using SystemError = SimError<3>;


             
class System;

class SystemFactory
{
public:
    virtual std::shared_ptr<System>create() = 0;
};

struct MemInfo {
    MEM_v                    mem;
    std::vector<std::string> cpu_names;
};

struct IOInfo {
    IO_v                     io;
    bool                     added;   // Indicate if this has been assigned.
    std::vector<std::string> cpu_names;
};

struct DevInfo {
    DEV_v                    dev;
    std::vector<std::string> io_names;
};

class System
{
public:
    System() {
 
    };
        
    virtual ~System() {};
    
    System(const System&) = delete;
    
 
    virtual auto getType() const -> std::string
    {
        return "System";
    }

    void showType()
    {
        std::cout << "Class Type = " << this->getType() << std::endl;
    }
    
    virtual size_t max_cpus() { return 1; }
    
    virtual size_t number_cpus() { return this->cpus.size(); }

    virtual void init();
    
    virtual void start();

   // virtual void shutdown();
   // virtual void run();
   // virtual void stop();

    // List all registered System model types.
    static
    void showModels() 
    {
        std::cout << " Registered models: " << std::endl;
        for(const auto& pair: factories)
            std::cout << " + " << pair.first << std::endl;
    }

    static void registerType(const std::string & name, SystemFactory *factory)
    {
        factories.insert(std::make_pair(name, factory));
    }
    
    static
    std::shared_ptr<System>create(const std::string &name)
    {
        if (factories.count(name) == 0)
            throw SystemError{"Unknown system type: " + name};
        return factories[name]->create();
    }
    
    void addCpu(CPU_v cpu)
    {
        this->cpus.push_back(cpu);
    }
    
    CPU_v& getCpu(size_t number)
    {
        if (this->cpus.size() > number)
            throw SystemError{"Not defined"};
        return this->cpus.at(number);
    }
    
    void addMemory(MemInfo mem)
    {
        this->memories.push_back(mem);
    }
    
    void addIo(IOInfo io)
    {
        this->io_ctrl.push_back(io);
    }
    
    void addDevice(DevInfo dev)
    {
        this->devices.push_back(dev);
    }

    virtual CPU_v create_cpu(const std::string &model) = 0;
    
    virtual MEM_v create_mem(const std::string &model, const size_t size, const size_t base = 0) = 0;
        
    virtual IO_v create_io(const std::string &model) = 0;

    virtual DEV_v create_dev(const std::string &model) = 0;

    std::vector<CPU_v> cpus;
    
    std::vector<IOInfo> io_ctrl;
    
    std::vector<MemInfo> memories;
    
    std::vector<DevInfo> devices;

private:
    static std::map<std::string, SystemFactory *> factories;
};

}
