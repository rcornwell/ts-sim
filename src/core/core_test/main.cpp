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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <map>
#include "System.h"
#include "CPU.h"
#include "../../src/core/Config.h"

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

using namespace std;

namespace core
{

class test : public System
{
public:
    test()
    {
    }
    virtual ~test()
    {
    }

    virtual void init()
    {

    }

    virtual auto getType() const -> string
    {
        return "test";
    }

    void showType()
    {
        cout << "Class Type = " << this->getType() << endl;
    }

    virtual size_t max_cpus()
    {
        return 1;
    }

//    static void registerCPU(const string & name, CPUFactory *factory)
//    {
//        cpu_factories.insert(make_pair(name, factory));
//    }
//
//    bool create_cpu(const string &model, int number)
//    {
//        if (number != 0)
//            return false;
//        cpu = cpu_factories[model]->create();
//        return true;
//    }
//
//    virtual CPU_v get_cpu(int number)
//    {
////        if (number != 0)
////            return nullptr;
//        return cpu;
//    }
//
//    static void registerIO(const string & name, IOFactory *factory)
//    {
//        io_factories.insert(make_pair(name, factory));
//    }
//
//    bool create_io(const string &model)
//    {
//        io = io_factories[model]->create();
//        return true;
//    }

    
    REGISTER_SYSTEM_TEMPLATE_CPU
    REGISTER_SYSTEM_TEMPLATE_IO
    REGISTER_SYSTEM_TEMPLATE_MEM
    
//private:
//    static map<string, CPUFactory *> cpu_factories;
//    static map<string, IOFactory *> io_factories;
//
//    CPU_v              cpu;
//    IO_v            io;

};
};

namespace emulator
{

enum cpu_model {
    s1, s2
};

template <enum cpu_model MOD>
class test_cpu : public CPU<uint32_t>
{
public:
    test_cpu() 
    {
    }
    
    virtual ~test_cpu()
    {
    }

    auto getType() const -> string
    {
        if constexpr (MOD == s1) return "s1";
        if constexpr (MOD == s2) return "s2";
    }
    
    virtual int max_cpus() { return 1; }
    
    bool   timer;
    int    home;

    virtual
    core::ConfigOptionParser options() {
        core::ConfigOptionParser option("CPU options");
        auto tim_opt = option.add<core::ConfigBool>("timer", "Optional timer", &timer);
        auto hom_opt = option.add<core::ConfigValue<int>>("home", "Home space offset", 0, &home);
        return option;
    }
};

template class test_cpu<s1>;
template class test_cpu<s2>;
}

namespace core {
map<string, SystemFactory *> System::factories;
REGISTER_SYSTEM(test);
map<string, CPUFactory *> test::cpu_factories;
map<string, IOFactory *> test::io_factories;
map<string, MemFactory *> test::mem_factories;
REGISTER_CPU(test, s1);
REGISTER_CPU(test, s2);
};

SUITE(ConfigLexer)
{

    TEST(Create) {
        istringstream ist{""};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

    TEST(Blanks) {
        istringstream ist{"   "};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

    TEST(System) {
        istringstream ist{" system"};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::Sys);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

    TEST(System2) {
        istringstream ist{"system test   "};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::Sys);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::Id);
        CHECK_EQUAL(lexer->token_text(), "test");
        delete lexer ;
    }

    TEST(Number) {
        vector<uint64_t> v { 12, 077, 0x40, 0xaf };
        istringstream ist{"12 077 0x40 0afh "};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        for(uint64_t i : v) {
            CHECK(lexer->token() == core::ConfigToken::Number);
            CHECK_EQUAL(lexer->token_value(), i);
            lexer->advance();
        }
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

    TEST(Tokens) {
        using namespace core;
        vector<ConfigToken> v {
            ConfigToken::Id, ConfigToken::Str, ConfigToken::Sys,
            ConfigToken::Cpu, ConfigToken::Dev, ConfigToken::Unit,
            ConfigToken::Ctl, ConfigToken::Units, ConfigToken::Number,
            ConfigToken::Rparn, ConfigToken::Lparn, ConfigToken::Colon,
            ConfigToken::Equal, ConfigToken::Comma, ConfigToken::Load,
            ConfigToken::Mount, ConfigToken::RO, ConfigToken::EOFSym
        };
        istringstream ist{"test \"test2\" system cpu device unit control units 032 ():=, load mount ro"};
        ConfigLexer *lexer = new ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        for(ConfigToken i : v) {
            CHECK(lexer->token() == i);
            lexer->advance();
        }
        CHECK(lexer->token() == ConfigToken::EOFSym);
        delete lexer;
    }
    
    TEST(Errors) {
        istringstream ist{"1a2 079 "};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        CHECK_THROW(lexer->advance(), core::Lexical_error);
        CHECK_THROW(lexer->advance(), core::Lexical_error);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }
}

SUITE(Config) {

    TEST(Start) {
        core::Config conf;
        string ist{"system test"};
        CHECK(conf(ist));
        CHECK_EQUAL(conf.sys->getType(), "test");
    }
    
    TEST(Start1) {
        core::Config conf;
        string ist{"system test2"};
        CHECK(!conf(ist));
        CHECK_EQUAL(conf.sys, nullptr);
    }

   TEST(Start2) {
        core::Config conf;
        string ist{"cpu test"};
        CHECK(!conf(ist));
    }
    
    TEST(CPUMake) {
        core::Config conf;
        string ist{"system test cpu s1:hello"};
        CHECK(conf(ist));
        CHECK_EQUAL(conf.sys->number_cpus(), 1u);
        core::CPU_v& cpu_v = conf.sys->cpus[0];
        auto caller = [](const auto& obj) { return obj->GetName(); };
        string name = std::visit(caller, cpu_v);
        CHECK_EQUAL(name, "hello");
        std::shared_ptr<emulator::CPU<uint32_t>> cpu_x = 
                std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
        std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
        CHECK_EQUAL(cpu->timer, false);
        CHECK_EQUAL(cpu->home, 0);
    }
    
    TEST(CPUOptions1) {
        core::Config conf;
        string ist{"system test cpu s2:hello2() "};
        CHECK(conf(ist));
        core::CPU_v& cpu_v = conf.sys->cpus[0];
        auto caller = [](const auto& obj) { return obj->GetName(); };
        string name = std::visit(caller, cpu_v);
        CHECK_EQUAL(name, "hello2");
        std::shared_ptr<emulator::CPU<uint32_t>> cpu_x = 
                std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
        std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
        CHECK_EQUAL(cpu->timer, false);
        CHECK_EQUAL(cpu->home, 0);
    }

    TEST(CPUOptions2) {
        core::Config conf;
        string ist{"system test cpu s1:opt_hello(timer,home=055) "};
        CHECK(conf(ist));
        core::CPU_v& cpu_v = conf.sys->cpus[0];
        auto caller = [](const auto& obj) { return obj->GetName(); };
        string name = std::visit(caller, cpu_v);
        CHECK_EQUAL(name, "opt_hello");
        std::shared_ptr<emulator::CPU<uint32_t>> cpu_x = 
                std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
        std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
        CHECK_EQUAL(cpu->timer, true);
        CHECK_EQUAL(cpu->home, 055);
    }
}


// run all tests
int main([[maybe_unused]]int argc, [[maybe_unused]]char **argv)
{
     return UnitTest::RunAllTests();
}
