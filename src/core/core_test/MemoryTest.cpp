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

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "Memory.h"
#include "RAM.h"
#include "ROM.h"
#include "CppUTest/TestHarness.h"

using namespace emulator;
using namespace std;

TEST_GROUP(MemoryTest)
{
};


TEST(MemoryTest, Create)
{
    // Create a RAM object and attempt to read and write it.
    // Compute access time based on read and write.
    shared_ptr<Memory<uint8_t>> mem = make_shared<RAM<uint8_t>>(4 * 1024, 0);
    Memory<uint8_t> *m = mem.get();
    int    i;
    auto start = chrono::high_resolution_clock::now();
    for (i = 0; i < 10000000; i++) {
        uint8_t  val = 0;
        if (!m->write(val, i & 0xfff))
            break;
        if (!m->read(val, i & 0xfff))
            break;
    }
    CHECK_EQUAL(10000000,i);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "RAM<uint8_t> raw ptr access time: " << (ctim.count() / (i * 2)) << " ns" << endl;
}

TEST(MemoryTest, Create32)
{
    // Do the same with a 32 bit wide peice of memory
    shared_ptr<Memory<uint32_t>> mem = make_shared<RAM<uint32_t>>(256 * 1024, 0);
    Memory<uint32_t> *m = mem.get();
    int    i;
    auto start = chrono::high_resolution_clock::now();
    for (i = 0; i < 10000000; i++) {
        uint32_t  val = 0;
        if (!m->write(val, i & 0x3ffff))
            break;
        if (!m->read(val, i & 0x3ffff))
            break;
    }
    CHECK_EQUAL(10000000, i);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "RAM<uint32_t> raw ptr large access time: " << (ctim.count() / (i*2)) << " ns" << endl;
}


TEST(MemoryTest, Create2)
{
    // Add in a Fixed size memory controller.
    shared_ptr<Memory<uint16_t>> memctl = make_shared<MemFixed<uint16_t>>(4 * 1024, 0);
    shared_ptr<Memory<uint16_t>> mem = make_shared<RAM<uint16_t>>(4 * 1024, 0);
    memctl->addMemory(mem);
    Memory<uint16_t> *m = memctl.get();
    int    i;
    auto start = chrono::high_resolution_clock::now();
    for (i = 0; i < 10000000; i++) {
        uint16_t  val = 0;
        if (!m->write(val, i & 0xfff))
            break;
        if (!m->read(val, i & 0xfff))
            break;
    }
    CHECK_EQUAL(10000000, i);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "Memory<uint16_t> fixed access time: " << (ctim.count() / (i*2)) << " ns" << endl;
}

TEST(MemoryTest, Create3)
{
    // Test Array controller with two pieces of RAM
    shared_ptr<Memory<uint16_t>> memctl = make_shared<MemArray<uint16_t>>(64 * 1024, 4096);
    shared_ptr<Memory<uint16_t>> mem = make_shared<RAM<uint16_t>>(32 * 1024, 0);
    shared_ptr<Memory<uint16_t>> mem2 = make_shared<RAM<uint16_t>>(32 * 1024, 32 * 1024);
    memctl->addMemory(mem);
    memctl->addMemory(mem2);
    Memory<uint16_t> *m = memctl.get();
    int    i;
    auto start = chrono::high_resolution_clock::now();
    for (i = 0; i < 10000000; i++) {
        uint16_t  val = 0;
        if (!m->write(val, i & 0xffff))
            break;
        if (!m->read(val, i & 0xffff))
            break;
    }
    CHECK_EQUAL(10000000, i);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "Memory<uint16_t> array access time: " << (ctim.count() / (i*2)) << " ns" << endl;
}


TEST(MemoryTest, Shared)
{
    // Test speed of accessing RAM object via a shared_ptr.
    shared_ptr<Memory<uint8_t>> mem = make_shared<RAM<uint8_t>>(64 * 1024, 0);
    int    i;
    auto start = chrono::high_resolution_clock::now();
    for (i = 0; i < 10000000; i++) {
        uint8_t  val = 0;
        if (!mem->write(val, i & 0xffff))
            break;
        if (!mem->read(val, i & 0xffff))
            break;
    }
    CHECK_EQUAL(10000000, i);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "RAM<uint8_t> shared ptr access time: " << (ctim.count() / (i * 2)) << " ns" << endl;
}

TEST(MemoryTest, Error)
{
    // Make sure accessing a RAM outsize it's size is an error.
    shared_ptr<Memory<uint8_t>> mem = make_shared<RAM<uint8_t>>(1024, 0);
    int    i;
    int    fail_count = 0;
    for (i = 0; i < 2048; i++) {
        uint8_t  val = 0;
        if (!mem->write(val, i)) {
            fail_count++;
        }
    }
    CHECK_EQUAL(2048, i);
    CHECK_EQUAL(1024, fail_count);
    fail_count = 0;
    for (i = 0; i < 2048; i++) {
        uint8_t val = 0xff;
        if (!mem->read(val, i)) {
            fail_count++;
        }
    }
    CHECK_EQUAL(2048, i);
    CHECK_EQUAL(1024, fail_count);
}

TEST(MemoryTest, Error2)
{
    // Make sure accessing a ROM outsize it's size is an error.
    shared_ptr<Memory<uint8_t>> mem = make_shared<ROM<uint8_t>>(1024, 0);
    int    i;
    int    fail_count = 0;
    for (i = 0; i < 2048; i++) {
        uint8_t  val = 0;
        if (!mem->write(val, i)) {
            fail_count++;
        }
    }
    CHECK_EQUAL(i, 2048);
    CHECK_EQUAL(fail_count, 1024);
    fail_count = 0;
    for (i = 0; i < 2048; i++) {
        uint8_t val = 0xff;
        if (!mem->read(val, i)) {
            fail_count++;
        }
    }
    CHECK_EQUAL(i, 2048);
    CHECK_EQUAL(fail_count, 1024);
}

TEST(MemoryTest, SetGet1)
{
    // Check that Set/Get works with RAM.
    shared_ptr<Memory<uint16_t>> mem = make_shared<RAM<uint16_t>>(1024, 0);
    int    i;
    for (i = 0; i < 1024; i++) {
        uint16_t  val = i;
        mem->Set(val, i);
    }
    CHECK_EQUAL(1024, i);
    for (i = 0; i < 1024; i++) {
        uint16_t val = 0xffff;
        mem->Get(val, i);
        CHECK_EQUAL(val, i);
    }
    CHECK_EQUAL(1024, i);
}

TEST(MemoryTest, SetGet2)
{
    shared_ptr<Memory<uint16_t>> mem = make_shared<RAM<uint16_t>>(1024, 0);
    uint16_t val = 0xff;
    CHECK_THROWS(emulator::Access_error, mem->Set(val, 2048));
    CHECK_THROWS(emulator::Access_error, mem->Get(val, 2048));
}

TEST(MemoryTest, ROM)
{
    // Make sure that ROM can't be modified.
    shared_ptr<Memory<uint16_t>> mem = make_shared<ROM<uint16_t>>(1024, 0);
    int    i;
    // Fill ROM with values.
    for (i = 0; i < 1024; i++) {
        uint16_t  val = 0xffff ^ i;
        mem->Set(val, i);
    }
    CHECK_EQUAL(1024, i);
    int fail_count = 0;
    // Attempt to modify the memory. Should not fail.
    for (i = 0; i < 1024; i++) {
        uint16_t val = i;
        if(!mem->write(val, i)) {
            fail_count++;
        }
    }
    CHECK_EQUAL(0, fail_count);
    fail_count = 0;
    // Make sure no change to values.
    for (i = 0; i < 1024; i++) {
        uint16_t val = 0xa5a5;  // Value that should not occur.
        if (!mem->read(val, i)) {
            fail_count++;
        }
        CHECK_EQUAL(0xffff ^ i, val);
    }
    CHECK_EQUAL(fail_count, 0);
}
