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
#include "i8080_system.h"
#include "i8080_cpu.h"
#include "i8080_con.h"
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
void load_mem(string name, shared_ptr<Memory<uint8_t>> mem)
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
    // Create top level system object.
    shared_ptr<core::System> sys = core::System::create("i8080");
    // Create CPU and some memory.
    core::CPU_v      cpu_v = sys->create_cpu("I8080");
    core::MEM_v      ram_v = sys->create_mem("RAM", 62*1024, 0);
    core::MEM_v      rom_v = sys->create_mem("ROM", 2048, 0xf800);
    core::DEV_v      con_v = sys->create_dev("2651");

    // Get pointers to specific classes, to simplify things.
    shared_ptr<CPU<uint8_t>> cpu = get<shared_ptr<CPU<uint8_t>>>(cpu_v);
    shared_ptr<i8080_cpu<I8080>> cpu_8 = dynamic_pointer_cast<i8080_cpu<I8080>>(cpu);
    shared_ptr<Memory<uint8_t>> rom_m = get<shared_ptr<Memory<uint8_t>>>(rom_v);
    // Set up chunk size for memory access.
    cpu_8->page_size = 2048;
    visit([](const auto& obj) {
        obj->setAddress(0x5c);
    }, con_v);
    // Set the names on the objects.
    core::MemInfo    ram_info{ram_v, {"cpu"}};
    core::MemInfo    rom_info{rom_v, {"cpu"}};
    core::DevInfo    con_info{con_v, {}};
    cpu->setName("cpu");
    // Connect things together.
    sys->addCpu(cpu_v);
    sys->addMemory(ram_info);
    sys->addMemory(rom_info);
    sys->addDevice(con_info);

    // Load rom with monitor.
    load_mem("gb01.bin", rom_m);
    // Final initialization.
    sys->init();
    cpu->setPC(0xf800);
    sys->start();
    // Inject halt opcode.
    visit([](const auto& obj) {
        obj->Set(0166, 0);
    }, ram_v);

    while(cpu->running) {
    //    cpu->trace();
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
