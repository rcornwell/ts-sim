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

    template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
    template<class... Ts> overload(Ts...) -> overload<Ts...>;
void System::init()
{
    // First call init on all CPU's.
	std::cerr << "System Init" << std::endl;
    for(auto &cpu : cpus ) {
        auto do_init = [](const auto& obj) { obj->init(); };
        std::visit(do_init, cpu);
            auto g_name = [](const auto& obj) {
                return obj->GetName();
            };
            std::string name = std::visit(g_name, cpu);
	std::cerr << "Init CPU" << name << std::endl;
    }
    
    // Next give each CPU it's memory.
    for(auto &mem : memories ) {
        // See if this CPU matches the names vector.
        for (auto &cpu : cpus ) {
	    // Grab name of this CPU.
            auto caller = [](const auto& obj) {
                return obj->GetName();
            };
            std::string name = std::visit(caller, cpu);
	    // If Memory belongs to this CPU, add it.
            if (mem.cpu_names.size() == 0 || 
                std::find(mem.cpu_names.begin(), mem.cpu_names.end(), name)
	       		!= mem.cpu_names.end()) {
  		// Assign memory based on type match
                std::visit(overload{
                    [](std::shared_ptr<emulator::CPU<uint8_t>> & c,
                            std::shared_ptr<emulator::Memory<uint8_t>> & m)
                                    { c->add_memory(m); },
                    [](std::shared_ptr<emulator::CPU<uint16_t>> & c,
                            std::shared_ptr<emulator::Memory<uint16_t>> & m)
                                    { c->add_memory(m); }, 
                    [](std::shared_ptr<emulator::CPU<uint32_t>> & c,
                            std::shared_ptr<emulator::Memory<uint32_t>> & m)
                                    { c->add_memory(m); },             
                    [](std::shared_ptr<emulator::CPU<uint64_t>> & c,
                            std::shared_ptr<emulator::Memory<uint64_t>> & m)
                                    { c->add_memory(m); },  
                    []([[maybe_unused]]auto  c, [[maybe_unused]]auto m) {}
                }, cpu, mem.mem);
	    }
        }
    }

    // Next give each CPU it's IO controllers.
    for(auto &io : io_ctrl ) {
        // See if this CPU matches the names vector.
        for (auto &cpu : cpus ) {
	    // Grab name of this CPU.
            auto caller = [](const auto& obj) {
                return obj->GetName();
            };
            std::string name = std::visit(caller, cpu);
	    // If Memory belongs to this CPU, add it.
            if (io.cpu_names.size() == 0 || 
                std::find(io.cpu_names.begin(), io.cpu_names.end(), name)
	       		!= io.cpu_names.end()) {
  		// Assign memory based on type match
                std::visit(overload{
                    [](std::shared_ptr<emulator::CPU<uint8_t>> & c,
                            std::shared_ptr<emulator::IO<uint8_t>> & i)
                                    { c->add_io(i); },
                    [](std::shared_ptr<emulator::CPU<uint16_t>> & c,
                            std::shared_ptr<emulator::IO<uint16_t>> & i)
                                    { c->add_io(i); }, 
                    [](std::shared_ptr<emulator::CPU<uint32_t>> & c,
                            std::shared_ptr<emulator::IO<uint32_t>> & i)
                                    { c->add_io(i); },             
                    [](std::shared_ptr<emulator::CPU<uint64_t>> & c,
                            std::shared_ptr<emulator::IO<uint64_t>> & i)
                                    { c->add_io(i); },  
                    []([[maybe_unused]]auto  c, [[maybe_unused]]auto i) {}
                }, cpu, io.io);
	    }
        }
    }

    // Now hook CPU's up to IO units.
    for(auto &cpu : cpus ) {
        auto do_init = [](const auto& obj) { obj->init_io(); };
        std::visit(do_init, cpu);
    }
    std::cerr << "Init done" << std::endl;
}
    
void System::start()
{
    // Call start on all CPU's.
    for(auto &cpu : cpus ) {
        auto caller = [](const auto& obj) { obj->start(); };
        std::visit(caller, cpu);
    }
    
}


}
