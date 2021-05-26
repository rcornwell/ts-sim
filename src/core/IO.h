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


    virtual void init() {};
    virtual void shutdown() {};
    virtual void start() {};
    virtual void reset() {};
    virtual void stop() {};
    virtual void step() {};
    virtual void run() {};
    
    virtual bool input(T &val, size_t port);
    virtual bool output(T val, size_t port);

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
    

namespace core {
    
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