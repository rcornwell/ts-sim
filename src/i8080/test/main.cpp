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

#include <UnitTest++.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "i8080_cpu.h"
#include "RAM.h"
#include "IO.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// To add a test, simply put the following code in the a .cpp file of your choice:
//
// =================================
// Simple Test
// =================================
//
//  TEST(YourTestName)
//  {
//  }
//
// The TEST macro contains enough machinery to turn this slightly odd-looking syntax into legal C++, and automatically register the test in a global list.
// This test list forms the basis of what is executed by RunAllTests().
//
// If you want to re-use a set of test data for more than one test, or provide setup/teardown for tests,
// you can use the TEST_FIXTURE macro instead. The macro requires that you pass it a class name that it will instantiate, so any setup and teardown code should be in its constructor and destructor.
//
//  struct SomeFixture
//  {
//    SomeFixture() { /* some setup */ }
//    ~SomeFixture() { /* some teardown */ }
//
//    int testData;
//  };
//
//  TEST_FIXTURE(SomeFixture, YourTestName)
//  {
//    int temp = testData;
//  }
//
// =================================
// Test Suites
// =================================
//
// Tests can be grouped into suites, using the SUITE macro. A suite serves as a namespace for test names, so that the same test name can be used in two difference contexts.
//
//  SUITE(YourSuiteName)
//  {
//    TEST(YourTestName)
//    {
//    }
//
//    TEST(YourOtherTestName)
//    {
//    }
//  }
//
// This will place the tests into a C++ namespace called YourSuiteName, and make the suite name available to UnitTest++.
// RunAllTests() can be called for a specific suite name, so you can use this to build named groups of tests to be run together.
// Note how members of the fixture are used as if they are a part of the test, since the macro-generated test class derives from the provided fixture class.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace emulator;
using namespace std;

string dir;

/**
 * @class bdos
 * @author rich
 * @date 21/05/21
 * @file main.cpp
 * @brief BDOS emulator for test framework.
 */
class bdos : public IO<uint8_t>
{
public:

    i8080_cpu<I8080>   *cpu;
    MemFixed<uint8_t>  *mem;

    virtual void init() {};
    virtual void shutdown() {};
    virtual void start() {};
    virtual void reset() {};
    virtual void stop() {};
    virtual void step() {};
    virtual void run() {};

    virtual bool input(uint8_t &val, size_t port)
    {
        val = 0;
        if (port != 0)
            return false;
        return true;
    }
    virtual bool output(uint8_t val, size_t port)
    {
        uint8_t data;
        uint16_t addr;
        if (port == 1) {
            switch(cpu->regs[C]) {
            case 9:   // output
                addr = cpu->regpair<DE>();
                do {
                    mem->read(data, addr++);
                    // data = mem[addr++] & 0x7f;
                    if (data != '$')
                        std::cout << data;
                } while (data != '$');
                break;
            case 2: // output
                data = cpu->regs[E] & 0x7f;
                std::cout << data;
                break;
            }
            return true;
        } else if (port == 2) {
	    std::cout << val;
	    return true;
	}
        return false;
    }

private:

};

//  5: 171         mov a,c
//  6: 376 002     cpi 2
// 10: 302 017 000 jnz .+4
// 13: 173         mov a,e
// 14: 323 001     out 1
// 16: 311         ret
// 17: 376 009     cpi 9
// 21: 300         rnz
// 22: 325         push d
// 23: 032         ldax d
// 24: 023         inx  d
// 25: 376 044     cpi '$'
// 27: 302 034 000 jnz .+2
// 32: 321         pop  d
// 33: 311         ret
// 34: 323 001     out 1
// 32: 303 023 000 jmp 
//
uint8_t bdos_buffer[] = {0171, 0376, 0002, 0302, 0017, 0000, 0173, 0323, 0002,
                         0311, 0376, 0011, 0300, 0325, 0032, 0023, 0376, 0044,
			 0302, 0034, 0000, 0321, 0311, 0323, 0002, 0303, 0023, 0000
};

/**
 * @brief Load a CPM .COM file into memory.
 * @param name - name of file to read from. Global dir and "/" will be preappended.
 * @param mem - Pointer to memory object to load into.
 */
void load_mem(string name, Memory<uint8_t> * mem)
{
    char *buffer;
    size_t size;
    string  fname = dir + "/" + name;

    ifstream file (fname, ios::in|ios::binary|ios::ate);
    size = file.tellg();
    buffer = new char [size];
    file.seekg(0, ios::beg);
    file.read(buffer,size);
    file.close();

    for(size_t i = 0; i < size; i++) {
        mem->write(buffer[i], i + 0x100);
    }
}

SUITE(CPU)
{
    TEST(CPU) {
        uint64_t  tim = 0;
        uint64_t  n_inst = 0;
        CPU<uint8_t>      *cpu;
        bdos      io;
        MemFixed<uint8_t> *mem{new MemFixed<uint8_t>(64*1024)};
        mem->add_memory(new RAM<uint8_t>(64 * 1024, 0));

        load_mem("TST8080.COM", mem);

        cpu = new i8080_cpu<I8080>();
        io.cpu = (i8080_cpu<I8080> *)cpu;
        io.mem = mem;
        cpu->SetMem(mem);
        cpu->SetIO(&io);
        cpu->start();
        cpu->SetPC(0x100);

        mem->write(0166, 0);    // Inject halt opcode.
//        mem->write(0323, 5);  // Output
//        mem->write(0001, 6);  // Not important.
//        mem->write(0xc9, 7);  // return instruction.
	for (size_t i = 0; i < sizeof(bdos_buffer); i++) {
	    mem->write(bdos_buffer[i], i+5);
	}

        auto start = chrono::high_resolution_clock::now();
        while(cpu->running) {
    //        cpu->trace();
            tim += cpu->step();
            n_inst++;
        }
        cout << endl;
        cpu->stop();
        auto end = chrono::high_resolution_clock::now();
        cout << "Simulated time: " << tim << endl;
        cout << "Excuted: " << n_inst << endl;
        auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
        cout << "Time: " << ctim.count() << " ns" << endl;
        cout << "Cycle time: " << (tim / ctim.count()) << " ns" << endl;
        cout << "Instruct time: " << (ctim.count() / n_inst) << " ns" << endl;
        CHECK_EQUAL (cpu->pc, 1u);
        delete cpu;
        delete mem;
    }

    TEST(CPUTEST) {
        uint64_t  tim = 0;
        uint64_t  n_inst = 0;
        CPU<uint8_t>      *cpu;
        bdos      io;
        MemFixed<uint8_t> *mem{new MemFixed<uint8_t>(64*1024)};
        mem->add_memory(new RAM<uint8_t>(64 * 1024, 0));

        load_mem("CPUTEST.COM", mem);
        cpu = new i8080_cpu<I8080>();
        io.cpu = (i8080_cpu<I8080> *)cpu;
        io.mem = mem;
        cpu->SetMem(mem);
        cpu->SetIO(&io);
        cpu->start();
        cpu->SetPC(0x100);

        mem->write(0166, 0);    // Inject halt opcode.
//        mem->write(0323, 5);  // Output
//        mem->write(0001, 6);  // Not important.
//        mem->write(0xc9, 7);  // return instruction.
	for (size_t i = 0; i < sizeof(bdos_buffer); i++) {
	    mem->write(bdos_buffer[i], i+5);
	}
        
        auto start = chrono::high_resolution_clock::now();
        while(cpu->running) {
            //cpu->trace();
            tim += cpu->step();
            n_inst++;
        }
        cout << endl;
        cpu->stop();
        auto end = chrono::high_resolution_clock::now();
        cout << "Simulated time: " << tim << endl;
        cout << "Excuted: " << n_inst << endl;
        auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
        cout << "Time: " << ctim.count() << " ns" << endl;
        cout << "Cycle time: " << (tim / ctim.count()) << " ns" << endl;
        cout << "Instruct time: " << (ctim.count() / n_inst) << " ns" << endl;
        CHECK_EQUAL (cpu->pc, 1u);
        delete cpu;
        delete mem;
    }

    TEST(CPUPRE) {
        uint64_t  tim = 0;
        uint64_t   n_inst = 0;
        CPU<uint8_t>      *cpu;
        bdos      io;
        MemFixed<uint8_t> *mem{new MemFixed<uint8_t>(64*1024)};
        mem->add_memory(new RAM<uint8_t>(64 * 1024, 0));

        load_mem("8080PRE.COM", mem);
        cpu = new i8080_cpu<I8080>();
        io.cpu = (i8080_cpu<I8080> *)cpu;
        io.mem = mem;
        cpu->SetMem(mem);
        cpu->SetIO(&io);
        cpu->start();
        cpu->SetPC(0x100);

        mem->write(0166, 0);    // Inject halt opcode.
//        mem->write(0323, 5);  // Output
//        mem->write(0001, 6);  // Not important.
//        mem->write(0xc9, 7);  // return instruction.
	for (size_t i = 0; i < sizeof(bdos_buffer); i++) {
	    mem->write(bdos_buffer[i], i+5);
	}
        
        auto start = chrono::high_resolution_clock::now();
        while(cpu->running) {
            //cpu->trace();
            tim += cpu->step();
            n_inst++;
        }
        cout << endl;
        cpu->stop();
        auto end = chrono::high_resolution_clock::now();
        cout << "Simulated time: " << tim << endl;
        cout << "Excuted: " << n_inst << endl;
        auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
               cout << "Time: " << ctim.count() << " ns" << endl;
        cout << "Cycle time: " << (tim / ctim.count()) << " ns" << endl;
       cout << "Instruct time: " << (ctim.count() / n_inst) << " ns" << endl;
        CHECK_EQUAL (cpu->pc, 1u);
        delete cpu;
        delete mem;
    }

    TEST(CPUExer) {
        uint64_t  tim = 0;
        uint64_t   n_inst = 0;
        CPU<uint8_t>      *cpu;
        bdos      io;
        MemFixed<uint8_t> *mem{new MemFixed<uint8_t>(64*1024)};
        mem->add_memory(new RAM<uint8_t>(64 * 1024, 0));

        load_mem("8080EXER.COM", mem);
        cpu = new i8080_cpu<I8080>();
        io.cpu = (i8080_cpu<I8080> *)cpu;
        io.mem = mem;
        cpu->SetMem(mem);
        cpu->SetIO(&io);
        cpu->start();
        cpu->SetPC(0x100);

        mem->write(0166, 0);    // Inject halt opcode.
        mem->write(0323, 5);  // Output
        mem->write(0001, 6);  // Not important.
        mem->write(0xc9, 7);  // return instruction.
//	for (size_t i = 0; i < sizeof(bdos_buffer); i++) {
//	    mem->write(bdos_buffer[i], i+5);
//	}

       auto start = chrono::high_resolution_clock::now();
        while(cpu->running) {
            // cpu->trace();
            tim += cpu->step();
            n_inst++;
        }
        cout << endl;
        cpu->stop();
         auto end = chrono::high_resolution_clock::now();
        cout << "Simulated time: " << tim << endl;
        cout << "Excuted: " << n_inst << endl;
        auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
               cout << "Time: " << ctim.count() << " ns" << endl;
        cout << "Cycle time: " << (tim / ctim.count()) << " ns" << endl;
        cout << "Instruct time: " << (ctim.count() / n_inst) << " ns" << endl;
        CHECK_EQUAL (cpu->pc, 1u);
        delete cpu;
        delete mem;
    }

    TEST(CPU85Exer) {
        uint64_t  tim = 0;
        uint64_t   n_inst = 0;
        CPU<uint8_t>      *cpu;
        bdos      io;
        MemFixed<uint8_t> *mem{new MemFixed<uint8_t>(64*1024)};
        mem->add_memory(new RAM<uint8_t>(64 * 1024, 0));

        load_mem("8085EXER.COM", mem);
        cpu = new i8080_cpu<I8085>();
        io.cpu = (i8080_cpu<I8080> *)cpu;
        io.mem = mem;
        cpu->SetMem(mem);
        cpu->SetIO(&io);
        cpu->start();
        cpu->SetPC(0x100);

        mem->write(0166, 0);    // Inject halt opcode.
        mem->write(0323, 5);  // Output
        mem->write(0001, 6);  // Not important.
        mem->write(0xc9, 7);  // return instruction.
//	for (size_t i = 0; i < sizeof(bdos_buffer); i++) {
//	    mem->write(bdos_buffer[i], i+5);
//	}

        auto start = chrono::high_resolution_clock::now();
        while(cpu->running) {
            // cpu->trace();
            tim += cpu->step();
            n_inst++;
        }
        cout << endl;
        cpu->stop();
         auto end = chrono::high_resolution_clock::now();
        cout << "Simulated time: " << tim << endl;
        cout << "Excuted: " << n_inst << endl;
        auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
        cout << "Time: " << ctim.count() << " ns" << endl;
        cout << "Cycle time: " << (tim / ctim.count()) << " ns" << endl;
        cout << "Instruct time: " << (ctim.count() / n_inst) << " ns" << endl;
        CHECK_EQUAL (cpu->pc, 1u);
        delete cpu;
        delete mem;
    }
}

// run all tests
int main([[maybe_unused]]int argc, [[maybe_unused]]char **argv)
{
    dir = argv[1];
    return UnitTest::RunAllTests();
}
