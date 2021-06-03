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

#include "config.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <variant>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "i8080_cpu.h"
#include "RAM.h"
#include "IO.h"
#include "ConfigOption.h"


using namespace emulator;
using namespace std;




/**
 * @brief Load a CPM .COM file into memory.
 * @param name - name of file to read from. Global dir and "/" will be preappended.
 * @param mem - Pointer to memory object to load into.
 */
void load_mem(string name, std::shared_ptr<Memory<uint8_t>> mem)
{
    char *buffer;
    size_t size;
    string  fname = name;

    ifstream file (fname, ios::in|ios::binary|ios::ate);
    size = file.tellg();
    buffer = new char [size];
    file.seekg(0, ios::beg);
    file.read(buffer,size);
    file.close();

    for(size_t i = 0; i < size; i++) {
        mem->Set(buffer[i], i);
    }
    delete[] buffer;
}


void test_system()
{
    uint64_t  tim = 0;
    uint64_t   n_inst = 0;
    std::shared_ptr<core::System> sys = core::System::create("i8080");
    core::CPU_v      cpu_v = sys->create_cpu("I8080");
    core::MEM_v      ram_v = sys->create_mem("RAM", 62*1024, 0);
    core::MEM_v      rom_v = sys->create_mem("ROM", 2048, 0xf800);
    std::shared_ptr<emulator::CPU<uint8_t>> cpu = 
	    std::get<std::shared_ptr<emulator::CPU<uint8_t>>>(cpu_v);
    std::shared_ptr<emulator::i8080_cpu<I8080>> cpu_8 =
        std::dynamic_pointer_cast<emulator::i8080_cpu<I8080>>(cpu);
    std::shared_ptr<emulator::Memory<uint8_t>> rom_m = 
            std::get<std::shared_ptr<emulator::Memory<uint8_t>>>(rom_v); 
    cpu_8->page_size = 2048;
    cpu->SetName("cpu");
    sys->add_cpu(cpu_v);
    core::MemInfo    ram_info{ram_v, {"cpu"}};
    core::MemInfo    rom_info{rom_v, {"cpu"}};
    sys->add_memory(ram_info);
    sys->add_memory(rom_info);
    auto caller = [](const auto& obj) { obj->Set(0166, 0); };
    std::visit(caller, ram_v);
    load_mem("gb01.bin", rom_m);
    sys->init();
    cpu->SetPC(0xf800);
    sys->start();
    // Inject halt opcode.
//        mem->write(0323, 5);  // Output
//        mem->write(0001, 6);  // Not important.
//        mem->write(0xc9, 7);  // return instruction.

    while(cpu->running) {
        cpu->trace();
        tim += cpu->step();
        n_inst++;
    }
    cout << endl;
    cpu->stop();
}

int main(int argc, char **argv)
{
	test_system();
	return 0;
}
