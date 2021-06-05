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

template<class... Ts> struct overload : Ts... {
    using Ts::operator()...;
};
template<class... Ts> overload(Ts...) -> overload<Ts...>;

void System::init()
{
    // First call init on all CPU's.
    cerr << "System Init" << endl;
    for(auto &cpu : cpus ) {
        visit([](const auto& obj) {
            obj->init();
        }, cpu);
        string name = visit([](const auto& obj) {
            return obj->GetName();
        }, cpu);
        cerr << "Init CPU: " << name << endl;
        // Check if the CPU needs any I/O controllers.
        bool need_io = visit([](const auto& obj) {
            return obj->noIO();
        }, cpu);

        if (need_io) {
            // Get the IO controller for this CPU.
            IOInfo info{};
            IO_v io_ctrl = visit([](const auto& obj) {
                return obj->GetIO();
            }, cpu);
            info.io = io_ctrl;
            info.cpu_names.push_back(name);
            info.added = true;
            // Add it in, but flag as added.
            add_io(info);
            string io_name = visit([](const auto& obj) {
                return obj->GetName();
            }, io_ctrl);
            cerr << "CPU IO: " << io_name << endl;
        }
    }

    // Next give each CPU it's memory.
    for(auto &mem : memories ) {
        // See if this CPU matches the names vector.
        for (auto &cpu : cpus ) {
            // Grab name of this CPU.
            auto caller = [](const auto& obj) {
                return obj->GetName();
            };
            string name = visit(caller, cpu);
            // If Memory belongs to this CPU, add it.
            if (mem.cpu_names.size() == 0 ||
                find(mem.cpu_names.begin(), mem.cpu_names.end(), name)
                != mem.cpu_names.end()) {
                // Assign memory based on type match
                visit(overload{
                    [](shared_ptr<CPU<uint8_t>> & c,
                       shared_ptr<Memory<uint8_t>> & m)
                    {
                        c->add_memory(m);
                    },
                    [](shared_ptr<CPU<uint16_t>> & c,
                       shared_ptr<Memory<uint16_t>> & m)
                    {
                        c->add_memory(m);
                    },
                    [](shared_ptr<CPU<uint32_t>> & c,
                       shared_ptr<Memory<uint32_t>> & m)
                    {
                        c->add_memory(m);
                    },
                    [](shared_ptr<CPU<uint64_t>> & c,
                       shared_ptr<Memory<uint64_t>> & m)
                    {
                        c->add_memory(m);
                    },
                    []([[maybe_unused]]auto & c, [[maybe_unused]]auto & m) {}
                }, cpu, mem.mem);
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
                return obj->GetName();
            }, cpu);
            // If Memory belongs to this CPU, add it.
            if (io.cpu_names.size() == 0 ||
                find(io.cpu_names.begin(), io.cpu_names.end(), name)
                != io.cpu_names.end()) {
                // Assign memory based on type match
                visit(overload{
                    [](shared_ptr<CPU<uint8_t>> & c,
                       shared_ptr<IO<uint8_t>> & i)
                    {
                        c->add_io(i);
                    },
                    [](shared_ptr<CPU<uint16_t>> & c,
                       shared_ptr<IO<uint16_t>> & i)
                    {
                        c->add_io(i);
                    },
                    [](shared_ptr<CPU<uint32_t>> & c,
                       shared_ptr<IO<uint32_t>> & i)
                    {
                        c->add_io(i);
                    },
                    [](shared_ptr<CPU<uint64_t>> & c,
                       shared_ptr<IO<uint64_t>> & i)
                    {
                        c->add_io(i);
                    },
                    []([[maybe_unused]]auto & c, [[maybe_unused]]auto & i) {}
                }, cpu, io.io);
            }
        }
    }

    // Now hook CPU's up to IO units.
    for(auto &cpu : cpus ) {
        auto do_init = [](const auto& obj) {
            obj->init_io();
        };
        visit(do_init, cpu);
    }

    // Now attach devices to their I/O controllers.
    for(auto &dev : devices ) {
        string dev_name = visit([](const auto& obj) { return obj->GetName(); }, dev.dev);
        // See if this CPU matches the names vector.
        for (auto &io : io_ctrl ) {
            // Grab name of this IO controller.
            string name = visit([](const auto& obj) {
                return obj->GetName();
            }, io.io);
            // If Device belongs to this controller add it.
            if (dev.io_names.size() == 0 ||
                find(dev.io_names.begin(), dev.io_names.end(), name)
                != dev.io_names.end()) {
                cerr << " Adding device: " << dev_name << " to " << name << endl;
                // Assign memory based on type match
                visit(overload{
                    [](shared_ptr<IO<uint8_t>> & i,
                       shared_ptr<Device<uint8_t>> & d)
                    {
                        i->add_device(d);
                    },
                    [](shared_ptr<IO<uint16_t>> & i,
                       shared_ptr<Device<uint16_t>> & d)
                    {
                        i->add_device(d);
                    },
                    [](shared_ptr<IO<uint32_t>> & i,
                       shared_ptr<Device<uint32_t>> & d)
                    {
                        i->add_device(d);
                    },
                    [](shared_ptr<IO<uint64_t>> & i,
                       shared_ptr<Device<uint64_t>> & d)
                    {
                        i->add_device(d);
                    },
                    []([[maybe_unused]]auto & i, [[maybe_unused]]auto & d) {}
                }, io.io, dev.dev);
            }
        }
    }

    // Next go through units and attach them to their respective devices.

    // Lastly call init on all IO controllers. These will then call init
    // on all attached devices. Which will intern init all attached units.
    for(auto &io : io_ctrl ) {
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
 



}
