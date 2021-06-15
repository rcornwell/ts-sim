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
#include <stdio.h>
#include <stdlib.h>
#include <map>
#include <vector>
#include "System.h"
#include "CPU.h"
#include "ConfigFile.h"
#include "CppUTest/TestHarness.h"

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

    REGISTER_SYSTEM_TEMPLATE_CPU
    REGISTER_SYSTEM_TEMPLATE_IO
    REGISTER_SYSTEM_TEMPLATE_MEM
    REGISTER_SYSTEM_TEMPLATE_DEV
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
        timer = false;
        home = 0;
    }

    virtual ~test_cpu()
    {
    }

    auto getType() const -> string
    {
        if constexpr (MOD == s1) return "s1";
        if constexpr (MOD == s2) return "s2";
    }

    virtual int max_cpus()
    {
        return 1;
    }

    bool   timer;
    int    home;

    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("CPU options");
        auto tim_opt = option.add<core::ConfigBool>("timer", "Optional timer", &timer);
        auto hom_opt = option.add<core::ConfigValue<int>>("home", "Home space offset", 0, &home);
        return option;
    }
};

template class test_cpu<s1>;
template class test_cpu<s2>;
}

namespace core
{
map<string, SystemFactory *> System::factories;
REGISTER_SYSTEM(test);
map<string, CPUFactory *> test::cpu_factories;
map<string, IOFactory *> test::io_factories;
map<string, MemFactory *> test::mem_factories;
map<string, DeviceFactory *> test::dev_factories;
REGISTER_CPU(test, s1);
REGISTER_CPU(test, s2);
};


TEST_GROUP(ConfigFile)
{
};

TEST(ConfigFile, Start)
{
    core::ConfigFile conf;
    string ist{"system test"};
    CHECK(conf(ist));
    CHECK_EQUAL(conf.sys->getType(), "test");
}

TEST(ConfigFile, Start1)
{
    core::ConfigFile conf;
    string ist{"system test2"};
    CHECK(!conf(ist));
    CHECK(conf.sys == nullptr);
}

TEST(ConfigFile, Start2)
{
    core::ConfigFile conf;
    string ist{"cpu test"};
    CHECK(!conf(ist));
}

TEST(ConfigFile, CPUMake)
{
    core::ConfigFile conf;
    string ist{"system test cpu s1:hello"};
    CHECK(conf(ist));
    CHECK_EQUAL(conf.sys->number_cpus(), 1u);
    core::CPU_v& cpu_v = conf.sys->cpus[0];
    string name = std::visit([](const auto& obj) {
        return obj->getName();
    }, cpu_v);
    CHECK_EQUAL(name, "hello");
    std::shared_ptr<emulator::CPU<uint32_t>> cpu_x =
            std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
    std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
    CHECK_EQUAL(cpu->timer, false);
    CHECK_EQUAL(cpu->home, 0);
}

TEST(ConfigFile, CPUOptions1)
{
    core::ConfigFile conf;
    string ist{"system test cpu s2:hello2() "};
    CHECK(conf(ist));
    core::CPU_v& cpu_v = conf.sys->cpus[0];
    string name = std::visit([](const auto& obj) {
        return obj->getName();
    }, cpu_v);
    CHECK_EQUAL(name, "hello2");
    std::shared_ptr<emulator::CPU<uint32_t>> cpu_x =
            std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
    std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
    CHECK_EQUAL(cpu->timer, false);
    CHECK_EQUAL(cpu->home, 0);
}

TEST(ConfigFile, CPUOptions2)
{
    core::ConfigFile conf;
    string ist{"system test cpu s1:opt_hello(timer,home=055) "};
    CHECK(conf(ist));
    core::CPU_v& cpu_v = conf.sys->cpus[0];
    string name = std::visit([](const auto& obj) {
        return obj->getName();
    }, cpu_v);
    CHECK_EQUAL(name, "opt_hello");
    std::shared_ptr<emulator::CPU<uint32_t>> cpu_x =
            std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
    std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
    CHECK_EQUAL(cpu->timer, true);
    CHECK_EQUAL(cpu->home, 055);
}

TEST(ConfigFile, CPUOptions3)
{
    core::ConfigFile conf;
    string ist{"system test"};
    string ist2{"cpu s1:opt_hello(home=057) "};
    CHECK(conf(ist));
    CHECK(conf(ist2));
    core::CPU_v& cpu_v = conf.sys->cpus[0];
    string name = std::visit([](const auto& obj) {
        return obj->getName();
    }, cpu_v);
    CHECK_EQUAL(name, "opt_hello");
    std::shared_ptr<emulator::CPU<uint32_t>> cpu_x =
            std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
    std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
    CHECK_EQUAL(cpu->timer, false);
    CHECK_EQUAL(cpu->home, 057);
}

TEST(ConfigFile, CPUOptions4)
{
    core::ConfigFile conf;
    string ist{"System test"};
    string ist2{"Cpu s1:Opt_hello(Home=057) "};
    CHECK(conf(ist));
    CHECK(conf(ist2));
    core::CPU_v& cpu_v = conf.sys->cpus[0];
    string name = std::visit([](const auto& obj) {
        return obj->getName();
    }, cpu_v);
    CHECK_EQUAL(name, "Opt_hello");
    std::shared_ptr<emulator::CPU<uint32_t>> cpu_x =
            std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
    std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
    CHECK_EQUAL(cpu->timer, false);
    CHECK_EQUAL(cpu->home, 057);
}


#if 0
TEST(ConfigFile, MemOptions1)
{
    core::ConfigFile conf;
    string ist1{"system test"};
    string ist2{"cpu s1:opt_hello(timer,home=055) "};
    string ist3{"memory m1:mem=opt_hello 64k ()"};
    CHECK(conf(ist1));
    CHECK(conf(ist2));
    CHECK(conf(ist3));
    core::CPU_v& cpu_v = conf.sys->cpus[0];
    string name = std::visit([](const auto& obj) {
        return obj->getName();
    }, cpu_v);
    CHECK_EQUAL(name, "opt_hello");
    std::shared_ptr<emulator::CPU<uint32_t>> cpu_x =
            std::get<shared_ptr<emulator::CPU<uint32_t>>>(cpu_v);
    std::shared_ptr<emulator::test_cpu<emulator::s2>> cpu =
                std::static_pointer_cast<emulator::test_cpu<emulator::s2>>(cpu_x);
    CHECK_EQUAL(cpu->timer, true);
    CHECK_EQUAL(cpu->home, 055);
}
#endif
