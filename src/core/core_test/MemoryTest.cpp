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


TEST(MemoryTest, Create) {
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
    CHECK_EQUAL(i, 10000000);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "Memory<uint8_t> access time: " << (ctim.count() / (i * 2)) << " ns" << endl;
    }
    
    
TEST(MemoryTest, Create32) {
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
    CHECK_EQUAL(i, 10000000);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "Memory<uint32_t> large access time: " << (ctim.count() / (i*2)) << " ns" << endl;
    }


TEST(MemoryTest, Create2) {
    shared_ptr<Memory<uint16_t>> memctl = make_shared<MemFixed<uint16_t>>(4 * 1024, 0);
    shared_ptr<Memory<uint16_t>> mem = make_shared<RAM<uint16_t>>(4 * 1024, 0);
    memctl->add_memory(mem);
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
    CHECK_EQUAL(i, 10000000);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "Memory<uint16_t> fixed access time: " << (ctim.count() / (i*2)) << " ns" << endl;
    }

TEST(MemoryTest, Create3) {
    shared_ptr<Memory<uint16_t>> memctl = make_shared<MemArray<uint16_t>>(64 * 1024, 4096);
    shared_ptr<Memory<uint16_t>> mem = make_shared<RAM<uint16_t>>(32 * 1024, 0);
    shared_ptr<Memory<uint16_t>> mem2 = make_shared<RAM<uint16_t>>(32 * 1024, 32 * 1024);
    memctl->add_memory(mem);
    memctl->add_memory(mem2);
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
    CHECK_EQUAL(i, 10000000);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "Memory<uint16_t> array access time: " << (ctim.count() / (i*2)) << " ns" << endl;
    }


TEST(MemoryTest, Shared) {
    shared_ptr<Memory<uint8_t>> mem = make_shared<RAM<uint8_t>>(64 * 1024, 0);
    //Memory<uint8_t> *m = mem.get();
    int    i;
    auto start = chrono::high_resolution_clock::now();
    for (i = 0; i < 10000000; i++) {
        uint8_t  val = 0;
        if (!mem->write(val, i & 0xffff))
            break;
        if (!mem->read(val, i & 0xffff))
            break;
    }
    CHECK_EQUAL(i, 10000000);
    auto end = chrono::high_resolution_clock::now();
    auto ctim = chrono::duration_cast<chrono::nanoseconds>(end - start);
    cout << "Memory<uint8_t> shared access time: " << (ctim.count() / (i * 2)) << " ns" << endl;
    }