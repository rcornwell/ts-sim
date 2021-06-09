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
#include <algorithm>
#include "System.h"

namespace core
{

using namespace std;
using namespace emulator;

void System::init()
{
    // First call init on all CPU's.
    cerr << "System Init" << endl;
    for(auto &cpu : cpus ) {
        visit([](const auto& obj) {
            obj->init();
        }, cpu);
        string name = visit([](const auto& obj) {
            return obj->getName();
        }, cpu);
        cerr << "Init CPU: " << name << endl;
        // Check if the CPU needs any I/O controllers.
        bool need_io = visit([](const auto& obj) {
            return obj->noIO();
        }, cpu);

        if (need_io) {
            // Get the IO controller for this CPU.
            IOInfo info{};
            IO_v io_ctrl_ = visit([](const auto& obj) {
                return obj->getIO();
            }, cpu);
            info.io = io_ctrl_;
            info.cpu_names.push_back(name);
            info.added = true;
            // Add it in, but flag as added.
            addIo(info);
            string io_name = visit([](const auto& obj) {
                return obj->getName();
            }, io_ctrl_);
            cerr << "CPU IO: " << io_name << endl;
        }
    }

    // Next give each CPU it's memory.
    for(auto &mem : memories ) {
        // See if this CPU matches the names vector.
        for (auto &cpu : cpus ) {
            // Grab name of this CPU.
            string name = visit([](const auto& obj) {
                return obj->getName();
            }, cpu);
            // If Memory belongs to this CPU, add it.
            if (mem.cpu_names.size() == 0 ||
                find(mem.cpu_names.begin(), mem.cpu_names.end(), name)
                != mem.cpu_names.end()) {
                attachMemory(cpu, mem.mem);
            }
        }
    }

    // Then give each CPU it's IO controllers.
    for(auto &io : io_ctrl ) {
        // Check if already belongs to a CPU.
        if (io.added) {
            continue;
        }
        // See if this CPU matches the names vector.
        for (auto &cpu : cpus ) {
            // Grab name of this CPU.
            string name = visit([](const auto& obj) {
                return obj->getName();
            }, cpu);
            // If Memory belongs to this CPU, add it.
            if (io.cpu_names.size() == 0 ||
                find(io.cpu_names.begin(), io.cpu_names.end(), name)
                != io.cpu_names.end()) {
                // Assign IO based on type match
                attachIO(cpu, io.io);
            }
        }
    }

    // Now hook CPU's up to IO units.
    for(auto &cpu : cpus ) {
        visit([](const auto& obj) {
            obj->init_io();
        }, cpu);
    }

    // Now attach devices to their I/O controllers.
    for(auto &dev : devices ) {
        string dev_name = visit([](const auto& obj) {
            return obj->getName();
        }, dev.dev);
        // See if this CPU matches the names vector.
        for (auto &io : io_ctrl ) {
            // Grab name of this IO controller.
            string name = visit([](const auto& obj) {
                return obj->getName();
            }, io.io);
            // If Device belongs to this controller add it.
            if (dev.io_names.size() == 0 ||
                find(dev.io_names.begin(), dev.io_names.end(), name)
                != dev.io_names.end()) {
                cerr << " Adding device: " << dev_name << " to " << name << endl;
                // Assign Device based on type match
                attachDevice(io.io, dev.dev);
            }
        }
    }

    // Next go through units and attach them to their respective devices.

    // Lastly call init on all IO controllers. These will then call init
    // on all attached devices. Which will intern init all attached units.
    for(auto &io : io_ctrl ) {
        string name = visit([](const auto& obj) {
            return obj->getName();
        }, io.io);
        cerr << "Init(" << name << ")" << endl;
        visit([](const auto& obj) {
            obj->init();
        }, io.io);
    }

    cerr << "Init done" << endl;
}

void System::start()
{
    // Call start on all CPU's.
    for(auto &cpu : cpus ) {
        auto caller = [](const auto& obj) {
            obj->start();
        };
        std::visit(caller, cpu);
    }
}


void System::attachMemory(CPU_v & cpu, MEM_v &mem)
{
    try {
        switch(cpu.index()) {
        case 0: {
            shared_ptr<CPU<uint8_t>> c8 = std::get<shared_ptr<CPU<uint8_t>>>(cpu);
            shared_ptr<Memory<uint8_t>> m8 = std::get<shared_ptr<Memory<uint8_t>>>(mem);
            c8->addMemory(m8);
            }
            break;
        case 1: {
            shared_ptr<CPU<uint16_t>> c16 = std::get<shared_ptr<CPU<uint16_t>>>(cpu);
            shared_ptr<Memory<uint16_t>> m16 = std::get<shared_ptr<Memory<uint16_t>>>(mem);
            c16->addMemory(m16);
            }
            break;
        case 2: {
            shared_ptr<CPU<uint32_t>> c32 = std::get<shared_ptr<CPU<uint32_t>>>(cpu);
            shared_ptr<Memory<uint32_t>> m32 = std::get<shared_ptr<Memory<uint32_t>>>(mem);
            c32->addMemory(m32);
            }
            break;
        case 3: {
            shared_ptr<CPU<uint64_t>> c64 = std::get<shared_ptr<CPU<uint64_t>>>(cpu);
            shared_ptr<Memory<uint64_t>> m64 = std::get<shared_ptr<Memory<uint64_t>>>(mem);
            c64->addMemory(m64);
            }
            break;
        }
    } catch(std::bad_variant_access const& ex) {
        std::cerr << "Invalid CPU/Memory combination." << std::endl;
    }
}



void System::attachIO(CPU_v & cpu, IO_v & io)
{
    try {
        switch(cpu.index()) {
        case 0: {
            shared_ptr<CPU<uint8_t>> c8 = std::get<shared_ptr<CPU<uint8_t>>>(cpu);
            shared_ptr<IO<uint8_t>> i8 = std::get<shared_ptr<IO<uint8_t>>>(io);
            c8->addIo(i8);
            }
            break;
        case 1: {
            shared_ptr<CPU<uint16_t>> c16 = std::get<shared_ptr<CPU<uint16_t>>>(cpu);
            shared_ptr<IO<uint16_t>> i16 = std::get<shared_ptr<IO<uint16_t>>>(io);
            c16->addIo(i16);
            }
            break;
        case 2: {
            shared_ptr<CPU<uint32_t>> c32 = std::get<shared_ptr<CPU<uint32_t>>>(cpu);
            shared_ptr<IO<uint32_t>> i32 = std::get<shared_ptr<IO<uint32_t>>>(io);
            c32->addIo(i32);
            }
            break;
        case 3: {
            shared_ptr<CPU<uint64_t>> c64 = std::get<shared_ptr<CPU<uint64_t>>>(cpu);
            shared_ptr<IO<uint64_t>> i64 = std::get<shared_ptr<IO<uint64_t>>>(io);
            c64->addIo(i64);
            }
            break;
        }
    } catch(std::bad_variant_access const& ex) {
        std::cerr << "Invalid CPU/IO combination." << std::endl;
    }
}

void System::attachDevice(IO_v & io, DEV_v & dev)
{
    try {
        switch(io.index()) {
        case 0: {
            shared_ptr<Device<uint8_t>> d8 = std::get<shared_ptr<Device<uint8_t>>>(dev);
            shared_ptr<IO<uint8_t>> i8 = std::get<shared_ptr<IO<uint8_t>>>(io);
            i8->addDevice(d8);
            }
            break;
        case 1: {
            shared_ptr<Device<uint16_t>> d16 = std::get<shared_ptr<Device<uint16_t>>>(dev);
            shared_ptr<IO<uint16_t>> i16 = std::get<shared_ptr<IO<uint16_t>>>(io);
            i16->addDevice(d16);
            }
            break;
        case 2: {
            shared_ptr<Device<uint32_t>> d32 = std::get<shared_ptr<Device<uint32_t>>>(dev);
            shared_ptr<IO<uint32_t>> i32 = std::get<shared_ptr<IO<uint32_t>>>(io);
            i32->addDevice(d32);
            }
            break;
        case 3: {
            shared_ptr<Device<uint64_t>> d64 = std::get<shared_ptr<Device<uint64_t>>>(dev);
            shared_ptr<IO<uint64_t>> i64 = std::get<shared_ptr<IO<uint64_t>>>(io);
            i64->addDevice(d64);
            }
            break;
        }
    } catch(std::bad_variant_access const& ex) {
        std::cerr << "Invalid CPU/Device combination." << std::endl;
    }
}


}
