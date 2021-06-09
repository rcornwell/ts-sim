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

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <utility>
#include "i8080_cpu.h"
#include "i8080_system.h"

namespace emulator
{

using namespace std;


enum opcode_type {
    OPR, LXI, REGX, RP0, REG2, ABS,
    REG, IMMR, MOV, SOPR, IMM, NUM
};

const int opcode_len[] = {
    1, 3, 1, 1, 1, 3, 1, 2, 1, 1, 2, 1
};

const uint8_t opcode_mask[] = {
    0377, 0317, 0317, 0317, 0357, 0377,
    0307, 0307, 0300, 0370, 0377, 0307
};

const string reg_pairs[] = {
    "B", "B", "D", "D", "H", "H", "SP", "PSW"
};

const string reg_names[] = {
    "B", "C", "D", "E", "H", "L", "M", "A"
};

#define SGN SIGN
#define SPF SIGN|PAR

static uint8_t const flag_table[256] = {
    /*    0    1    2    3    4    5    6    7 */
    /* 00x */ PAR|ZERO,0, 0, PAR,   0, PAR, PAR,   0,
    /* 01x */   0, PAR, PAR,   0, PAR,   0,   0, PAR,
    /* 02x */   0, PAR, PAR,   0, PAR,   0,   0, PAR,
    /* 03x */ PAR,   0,   0, PAR,   0, PAR, PAR,   0,
    /* 04x */   0, PAR, PAR,   0, PAR,   0,   0, PAR,
    /* 05x */ PAR,   0,   0, PAR,   0, PAR, PAR,   0,
    /* 06x */ PAR,   0,   0, PAR,   0, PAR, PAR,   0,
    /* 07x */   0, PAR, PAR,   0, PAR,   0,   0, PAR,
    /* 10x */   0, PAR, PAR,   0, PAR,   0,   0, PAR,
    /* 11x */ PAR,   0,   0, PAR,   0, PAR, PAR,   0,
    /* 12x */ PAR,   0,   0, PAR,   0, PAR, PAR,   0,
    /* 13x */   0, PAR, PAR,   0, PAR,   0,   0, PAR,
    /* 14x */ PAR,   0,   0, PAR,   0, PAR, PAR,   0,
    /* 15x */   0, PAR, PAR,   0, PAR,   0,   0, PAR,
    /* 16x */   0, PAR, PAR,   0, PAR,   0,   0, PAR,
    /* 17x */ PAR,   0,   0, PAR,   0, PAR, PAR,   0,
    /* 20x */ SGN, SPF, SPF, SGN, SPF, SGN, SGN, SPF,
    /* 21x */ SPF, SGN, SGN, SPF, SGN, SPF, SPF, SGN,
    /* 22x */ SPF, SGN, SGN, SPF, SGN, SPF, SPF, SGN,
    /* 23x */ SGN, SPF, SPF, SGN, SPF, SGN, SGN, SPF,
    /* 24x */ SPF, SGN, SGN, SPF, SGN, SPF, SPF, SGN,
    /* 25x */ SGN, SPF, SPF, SGN, SPF, SGN, SGN, SPF,
    /* 26x */ SGN, SPF, SPF, SGN, SPF, SGN, SGN, SPF,
    /* 27x */ SPF, SGN, SGN, SPF, SGN, SPF, SPF, SGN,
    /* 30x */ SPF, SGN, SGN, SPF, SGN, SPF, SPF, SGN,
    /* 31x */ SGN, SPF, SPF, SGN, SPF, SGN, SGN, SPF,
    /* 32x */ SGN, SPF, SPF, SGN, SPF, SGN, SGN, SPF,
    /* 33x */ SPF, SGN, SGN, SPF, SGN, SPF, SPF, SGN,
    /* 34x */ SGN, SPF, SPF, SGN, SPF, SGN, SGN, SPF,
    /* 35x */ SPF, SGN, SGN, SPF, SGN, SPF, SPF, SGN,
    /* 36x */ SPF, SGN, SGN, SPF, SGN, SPF, SPF, SGN,
    /* 37x */ SGN, SPF, SPF, SGN, SPF, SGN, SGN, SPF
};

template <cpu_model MOD>
inline uint8_t i8080_cpu<MOD>::flag_gen(uint8_t v)
{
    if constexpr (MOD == I8085)
        return flag_table[v];
    else
        return flag_table[v] | VFLG;
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_add(uint8_t v)
{
    uint16_t  t;
    uint8_t   a = fetch_reg<A>();
    uint8_t   ac;
    uint8_t   c;

    ac = (a & 0xf) + (v & 0xf);
    t = (uint16_t)a + (uint16_t)v;
    c = (t & 0x100) != 0;
    t &= 0xff;
    PSW = flag_gen(t) | (ac & 0x10) | c;
    if constexpr (MOD == I8085) {
        if ((((a&v) | (a&t) | (v&t)) & SIGN) != 0)
            PSW |= XFLG;
        if ((((a & v & ~t) | (~a & ~v & t)) & SIGN) != 0)
            PSW |= VFLG;
    }
    set_reg<A>(t);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_adc(uint8_t v)
{
    uint8_t   a = fetch_reg<A>();
    uint8_t   c = PSW & CARRY;
    uint16_t  t;
    uint8_t   ac;

    ac = (a & 0xf) + (v & 0xf) + c;
    t = (uint16_t)a + (uint16_t)v + c;
    c = (t & 0x100) != 0;
    t &= 0xff;
    PSW = flag_gen(t) | (ac & 0x10) | c;
    if constexpr (MOD == I8085) {
        if ((((a&v) | (a&t) | (v&t)) & SIGN) != 0)
            PSW |= XFLG;
        if ((((a & v & ~t) | (~a & ~v & t)) & SIGN) != 0)
            PSW |= VFLG;
    }
    set_reg<A>(t);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_sub(uint8_t v)
{
    uint8_t   a = fetch_reg<A>();
    uint16_t  t;
    uint8_t   ac;
    uint8_t   c;

    v ^= 0xff;
    ac = (a & 0xf) + (v & 0xf) + 1;
    t = (uint16_t)a + (uint16_t)v + 1;
    c = (t & 0x100) == 0;
    t &= 0xff;
    PSW = flag_gen(t) | (ac & 0x10) | c;
    if constexpr (MOD == I8085) {
        if ((((a&~v) | (t&a) | (t&~v)) & SIGN) != 0)
            PSW |= XFLG;
        if ((((a & v & ~t) | (~a & ~v & t)) & SIGN) != 0)
            PSW |= VFLG;
    }
    set_reg<A>(t);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_sbb(uint8_t v)
{
    uint8_t   a = fetch_reg<A>();
    uint16_t  t;
    uint8_t   ac;
    uint8_t   c;

    c= !(PSW & CARRY);
    v ^= 0xff;
    ac = (a & 0xf) + (v & 0xf) + c;
    t = (uint16_t)a + (uint16_t)v + c;
    c = (t & 0x100) == 0;
    t &= 0xff;
    PSW = flag_gen(t) | (ac & 0x10) | c;
    if constexpr (MOD == I8085) {
        if ((((a&~v) | (t&a) | (t&~v)) & SIGN) != 0)
            PSW |= XFLG;
        if ((((a & v & ~t) | (~a & ~v & t)) & SIGN) != 0)
            PSW |= VFLG;
    }
    set_reg<A>(t);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_ana(uint8_t v)
{
    uint8_t  a = fetch_reg<A>();
    uint8_t  t;
    uint8_t  ac;

    if constexpr (MOD == I8080)
        ac = (a | v) << 1;
    if constexpr (MOD == I8085)
        ac = AC;
    t = a & v;
    PSW = flag_gen(t) | (ac & 0x10);
    if constexpr (MOD == I8085) {
        if ((((a&v) | (t&a) | (t&v)) & SIGN) != 0)
            PSW |= XFLG;
    }
    set_reg<A>(t);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_xra(uint8_t v)
{
    uint8_t  t;
    uint8_t  a = fetch_reg<A>();

    t = a ^ v;
    if constexpr (MOD == I8080)
        PSW = 0;
    if constexpr (MOD == I8085) {
        PSW = VFLG & PSW;
        if ((((a&v) | (t&a) | (t&v)) & SIGN) != 0)
            PSW |= XFLG;
    }
    PSW |= flag_gen(t);
    set_reg<A>(t);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_ora(uint8_t v)
{
    uint8_t  a =fetch_reg<A>();
    uint8_t  t;

    t = a | v;
    PSW = flag_gen(t);
    if constexpr (MOD == I8085) {
        if ((((a&v) | (t&a) | (t&v)) & SIGN) != 0)
            PSW |= XFLG;
    }
    set_reg<A>(t);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_cmp(uint8_t v)
{
    uint8_t   a = fetch_reg<A>();
    uint16_t  t;
    uint8_t   ac;
    uint8_t   c;

    v ^= 0xff;
    ac = (a & 0xf) + (v & 0xf) + 1;
    t = (uint16_t)a + (uint16_t)v + 1;
    c = (t & 0x100) == 0;
    t &= 0xff;
    PSW = flag_gen(t) | (ac & 0x10) | c;
    if constexpr (MOD == I8085) {
        if ((((a&~v) | (t&a) | (t&~v)) & SIGN) != 0)
            PSW |= XFLG;
        if ((((a & v & ~t) | (~a & ~v & t)) & SIGN) != 0)
            PSW |= VFLG;
    }

}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_daa()
{
    uint8_t   a = fetch_reg<A>();
    uint16_t  d = 0;
    uint8_t   c = 0;
    uint16_t  t;
    uint8_t   ac = 0;


    if ((PSW & AC) != 0 || (a & 0xf) > 9) {
        d += 0x6;
        ac = ((a & 0xf) > 9) ? AC : 0;
    }
    if ((PSW & CARRY) != 0 || a >= 0x9A) {
        d += 0x60;
        c = 1;
    }
    t = (a + d) & 0xff;
    PSW = flag_gen(t) | (ac & 0x10) | c;
    set_reg<A>(t);
}

template <cpu_model MOD>
template <reg_name R>
inline void i8080_cpu<MOD>::o_inr()
{
    uint8_t r = fetch_reg<R>();
    uint8_t t = (uint16_t)(r) + 1;
    uint8_t ac = ((t & 0xf) == 0) ? AC : 0;

    PSW &= CARRY;
    PSW |= flag_gen(t) | ac;
    set_reg<R>(t);
}

template <cpu_model MOD>
template <reg_name R>
inline void i8080_cpu<MOD>::o_dcr()
{
    uint8_t   r = fetch_reg<R>();
    uint8_t   t = r + 0xff;
    uint8_t   ac = ((t & 0xf) == 0xf) ? 0: AC;

    PSW &= CARRY;
    PSW |= flag_gen(t) | (ac & 0x10);
    set_reg<R>(t);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_adi()
{
    uint8_t data;
    data = fetch();
    o_add(data);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_aci()
{
    uint8_t data;
    data = fetch();
    o_adc(data);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_sui()
{
    uint8_t data;
    data = fetch();
    o_sub(data);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_sbi()
{
    uint8_t data;
    data = fetch();
    o_sbb(data);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_ani()
{
    uint8_t data;
    data = fetch();
    o_ana(data);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_xri()
{
    uint8_t data;
    data = fetch();
    o_xra(data);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_ori()
{
    uint8_t data;
    data = fetch();
    o_ora(data);
}

template <cpu_model MOD>
inline void i8080_cpu<MOD>::o_cpi()
{
    uint8_t data;
    data = fetch();
    o_cmp(data);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_rlc()
{
    uint8_t  c;
    uint8_t  a = fetch_reg<A>();
    c = a >> 7;
    a = (a << 1) | c;
    PSW = (PSW & ~CARRY) | c;
    set_reg<A>(a);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_rrc()
{
    uint8_t c;
    uint8_t a = fetch_reg<A>();
    c = (a & 1);
    a >>= 1;
    if (c)
        a |= SIGN;
    set_reg<A>(a);
    PSW = (PSW & ~CARRY) | c;
    if constexpr (MOD == I8085) {
        PSW &= ~VFLG;
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_ral()
{
    uint8_t c;
    uint8_t a = fetch_reg<A>();

    c = a >> 7;
    a = (a << 1) | (PSW & CARRY);
    set_reg<A>(a);
    PSW = (PSW & ~CARRY) | c;
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_rar()
{
    uint8_t c;
    uint8_t a = fetch_reg<A>();

    c = a & 1;
    a >>= 1;
    if (PSW & CARRY)
        a |= SIGN;
    set_reg<A>(a);
    PSW = (PSW & ~CARRY) | c;
    if constexpr (MOD == I8085) {
        PSW &= ~VFLG;
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_cma()
{
    uint8_t a = fetch_reg<A>();
    a ^= 0377;
    set_reg<A>(a);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_stc()
{
    PSW |= CARRY;
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_cmc()
{
    PSW ^= CARRY;
}

template <cpu_model MOD>
template <reg_pair RP>
void i8080_cpu<MOD>::o_lxi()
{
    uint16_t addr;

    addr = fetch_addr();
    setregpair<RP>(addr);
}

template <cpu_model MOD>
template <reg_pair RP>
void i8080_cpu<MOD>::o_dad()
{
    uint32_t t;
    t = (uint32_t)regpair<HL>() + (uint32_t)regpair<RP>();
    setregpair<HL>(t & 0xffff);
    PSW &= ~CARRY;
    if (t &  0x10000)
        PSW |= CARRY;
}

template <cpu_model MOD>
template <reg_pair RP>
void i8080_cpu<MOD>::o_inx()
{
    uint16_t addr;

    addr = regpair<RP>();
    setregpair<RP>(addr + 1);
}

template <cpu_model MOD>
template <reg_pair RP>
void i8080_cpu<MOD>::o_dcx()
{
    uint16_t addr;

    addr = regpair<RP>();
    setregpair<RP>(addr - 1);
}

template <cpu_model MOD>
template <reg_pair RP>
void i8080_cpu<MOD>::o_stax()
{
    uint16_t addr;

    addr = regpair<RP>();
    mem->write(regs[A], addr);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_lhld()
{
    uint16_t addr;

    addr = fetch_addr();
    addr = fetch_double(addr);
    setregpair<HL>(addr);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_shld()
{
    uint16_t addr;
    addr = fetch_addr();
    store_double(regpair<HL>(), addr);
}

template <cpu_model MOD>
template <reg_pair RP>
void i8080_cpu<MOD>::o_ldax()
{
    uint16_t addr;

    addr = regpair<RP>();
    mem->read(regs[A], addr);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_sta()
{
    uint16_t addr;

    addr = fetch_addr();
    mem->write(regs[A], addr);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_lda()
{
    uint16_t addr;
    uint8_t data;

    addr = fetch_addr();
    mem->read(data, addr);
    regs[A] = data;
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_rcc(int c)
{
    if (c) {
        pc = pop();
        cycle_time += 6*Tc;
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_ccc(int c)
{
    uint16_t addr = fetch_addr();

    if (c) {
        push(pc);
        pc = addr;
        cycle_time += 6*Tc;
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_jcc(int c)
{
    uint16_t addr = fetch_addr();

    if (c) {
        pc = addr;

    }
}

template <cpu_model MOD>
template <reg_pair RP>
void i8080_cpu<MOD>::o_pop()
{
    uint16_t addr;

    addr = pop();
    setregpair<RP>(addr);
}

template <cpu_model MOD>
template <reg_pair RP>
void i8080_cpu<MOD>::o_push()
{
    uint16_t addr;
    addr = regpair<RP>();
    push(addr);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_call()
{
    uint16_t addr;

    addr = fetch_addr();
    push(pc);
    pc = addr;
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_jmp()
{
    uint16_t addr;
    addr = fetch_addr();
    pc = addr;
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_xcht()
{
    uint8_t data;
    uint8_t r;

    r = fetch_reg<L>();
    mem->read(data, sp);
    mem->write(r, sp);
    set_reg<L>(data);
    r = fetch_reg<H>();
    mem->read(data, sp+1);
    mem->write(r, sp+1);
    set_reg<H>(data);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_xchg()
{
    uint16_t addr;

    addr = regpair<HL>();
    setregpair<HL>(regpair<DE>());
    setregpair<DE>(addr);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_out()
{
    uint8_t    port;
    port = fetch();
    io->output(regs[A], port);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_ret()
{
    pc = pop();
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_pchl()
{
    pc = regpair<HL>();
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_sphl()
{
    sp = regpair<HL>();
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_in()
{
    uint8_t data;
    data = fetch();
    io->input(regs[A], data);
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_di()
{
    ie = false;
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_ei()
{
    ie = true;
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_hlt()
{
    running = false;
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_nop()
{
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_rim()
{
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_sim()
{
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_dsub()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint32_t  t;
        t = (uint32_t)regpair<HL>() - (uint32_t)regpair<BC>();
        setregpair<HL>(t & 0xffff);
        PSW &= ~CARRY;
        if (t &  0x10000)
            PSW |= CARRY;
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_arhl()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint16_t t;
        t = regpair<HL>();
        PSW &= ~CARRY;
        PSW |= t & CARRY;
        t = (t & 0x8000) | (t >> 1);
        setregpair<HL>(t);
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_rdel()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint16_t t;
        uint16_t c = PSW & CARRY;
        t = regpair<DE>();
        PSW &= ~CARRY;
        if (t & 0x8000)
            PSW |= t & CARRY;
        t = (t << 1) | c;
        setregpair<DE>(t);
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_ldhi()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint16_t t = regpair<HL>();
        uint8_t  data = fetch();

        t += (uint16_t) data;
        setregpair<DE>(t);
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_ldsi()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint16_t t = regpair<SP>();
        uint8_t  data = fetch();

        t += (uint16_t) data;
        setregpair<DE>(t);
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_rstv()
{
    if constexpr (MOD == cpu_model::I8085) {
        if (PSW & VFLG) {
            push(pc);
            pc = 0x40;
        }
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_shlx()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint16_t  data = regpair<HL>();
        uint16_t  addr = regpair<DE>();

        store_double(data, addr);
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_lhlx()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint16_t  addr = regpair<DE>();
        uint16_t  data = fetch_double(addr);

        setregpair<HL>(data);
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_jnx5()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint16_t addr = fetch_addr();

        if ((PSW & XFLG) == 0)
            pc = addr;
    }
}

template <cpu_model MOD>
void i8080_cpu<MOD>::o_jx5()
{
    if constexpr (MOD == cpu_model::I8085) {
        uint16_t addr = fetch_addr();

        if ((PSW & XFLG) != 0)
            pc = addr;
    }
}

#undef INSN
#define OPR(f,b)       case b: o_##f(); break;
#define ABS(f,b) OPR(f,b)
#define OREGX(f,b,r)   case (((int)r) << 3) + b: o_##f<r>(); break;
#define REG(f,b) OREGX(f,b,B) OREGX(f,b,C) \
    OREGX(f,b,D) OREGX(f,b,E) \
    OREGX(f,b,H) OREGX(f,b,L) \
    OREGX(f,b,M) OREGX(f,b,A)
#define REGXO(f,b,r,x) case (x << 3) + b: o_##f<r>(); break;
#define REGX(f,b) REGXO(f,b,BC,0) REGXO(f,b,DE,2) \
    REGXO(f,b,HL,4) REGXO(f,b,SP,6)
#define REGP(f,b) REGXO(f,b,BC,0) REGXO(f,b,DE,2) \
    REGXO(f,b,HL,4) REGXO(f,b,PW,6)
#define LXI(f,b)  REGX(f,b)
#define IREGX(f,b,r)   case (((int)r) << 3) + b: \
    data = fetch(); set_reg<r>(data); break;
#define IREG(f,b) IREGX(f,b,B) IREGX(f,b,C) \
    IREGX(f,b,D) IREGX(f,b,E) \
    IREGX(f,b,H) IREGX(f,b,L) \
    IREGX(f,b,M) IREGX(f,b,A)
#define IMMR(f,b) IREG(f,b)
#define IMM(f,b) OPR(f,b)
#define REG2(f,b) REGXO(f,b,BC,0) REGXO(f,b,DE,2)
#define MSR(d,s,b)     case ((int)d<<3)+(int)s + b: \
    data = fetch_reg<s>(); set_reg<d>(data);  break;
#define MS(s,b) MSR(s,B,b) MSR(s,C,b) MSR(s,D,b) MSR(s,E,b) \
    MSR(s,H,b) MSR(s,L,b) MSR(s,M,b) MSR(s,A,b)
#define MSX(b) MSR(M,B,b) MSR(M,C,b) MSR(M,D,b) MSR(M,E,b) \
    MSR(M,H,b) MSR(M,L,b) MSR(M,A,b)
#define MOV(f,b) MS(B,b) MS(C,b) MS(D,b) MS(E,b) \
    MS(H,b) MS(L,b) MSX(b) MS(A,b)
#define OPS(a,f,b)      case a+b: data = fetch_reg<b>(); o_##f(data); break;
#define SOPR(f,a) OPS(a,f,B) OPS(a,f,C) OPS(a,f,D) OPS(a,f,E) \
    OPS(a,f,H) OPS(a,f,L) OPS(a,f,M) OPS(a,f,A)
#define CCX(f,b,n,cc,flag,test,t,m) case b + (n<<3): \
    o_##f((PSW & flag) test 0); break;
#define CCR(f,b) CC(f,OPR,b,0,cpu_model::I8080)
#define CCJ(f,b) CC(f,ABS,b,0,cpu_model::I8080)
#define CCC(f,b) CC(f,ABS,b,0,cpu_model::I8080)
#define RSTX(b,n)      case b+(n<<3): push(pc); pc = (n << 3); break;
#define RST(f,b) RSTX(b,0) RSTX(b,1) RSTX(b,2) RSTX(b,3) \
    RSTX(b,4) RSTX(b,5) RSTX(b,6) RSTX(b,7)
#define INSN(name, type, base, model) type(name, base)

template <cpu_model MOD>
void i8080_cpu<MOD>::decode(uint8_t op)
{
    uint8_t    data;

    switch(op) {
#include "../i8080/i8080_insn.h"
    }
}

template <cpu_model MOD>
uint64_t i8080_cpu<MOD>::step()
{
    uint8_t   ir;

    ir = fetch();
    cycle_time = ins_time[ir];
    decode(ir);
    io->step();

    return cycle_time;
}

#undef OPR
#undef ABS
#undef REG
#undef REGX
#undef REGP
#undef LXI
#undef IMMR
#undef IMM
#undef REG2
#undef MOV
#undef SOPR
#undef CCX
#undef CCR
#undef CCJ
#undef CCC
#undef RST
#undef INSN

string toUpper(string str)
{
    for (auto& c : str) {
        c = static_cast<char>(std::toupper(c));
    }

    return str;
}

#undef INSN
#define CCR(f,b,m)  CC(f,OPR,b,0,m)
#define CCJ(f,b,m)  CC(f,ABS,b,0,m)
#define CCC(f,b,m)  CC(f,ABS,b,0,m)
#define OPR(f,b,m)  { toUpper(#f), OPR, b, m },
#define LXI(f,b,m)  { toUpper(#f), LXI, b, m},
#define REGX(f,b,m) { toUpper(#f), REGX, b, m},
#define REG2(f,b,m) { toUpper(#f), REG2, b, m},
#define REGP(f,b,m) { toUpper(#f), RP0, b, m},
#define ABS(f,b,m)  { toUpper(#f), ABS, b, m},
#define REG(f,b,m)  { toUpper(#f), REG, b, m},
#define IMMR(f,b,m) { toUpper(#f), IMMR, b, m},
#define MOV(f,b,m)  { toUpper(#f), MOV, b, m},
#define SOPR(f,b,m) { toUpper(#f), SOPR, b, m},
#define IMM(f,b,m)  { toUpper(#f), IMM, b, m},
#define RST(f,b,m)  { toUpper(#f), NUM, b, m},
#define CCX(f,b,n,cc,flag,test,t,m) { toUpper(#cc), t, b+(n<<3), m },

#define INSN(name, type, base, mod) type(name, base, mod)

struct opcode {
    string       name;
    opcode_type  type;
    uint8_t      base;
    cpu_model    model;
} opcode_map[] = {
#include "../i8080/i8080_insn.h"
    { "", OPR, 0, I8080}
};

template <cpu_model MOD>
string i8080_cpu<MOD>::disassemble(uint8_t ir, uint16_t addr, int &len)
{
    const struct opcode *op;
    stringstream temp;

    len = 1;
    for(op = opcode_map; op->name != ""; op++) {
        if ((ir & opcode_mask[op->type]) == op->base)
            break;
    }
    if (op->name == "") {
        temp << hex << internal << setw(2) << setfill('0') << (unsigned int)(ir) << " ";
        return temp.str();
    }
    switch(op->type) {
    case OPR:
        temp << op->name;
        break;
    case LXI:
        temp << op->name << " " << reg_pairs[(ir >> 3) & 06] <<
             "," << std::hex << addr;
        len = 3;
        break;
    case REGX:
        temp << op->name << " " << reg_pairs[(ir >> 3) & 06];
        break;
    case RP0:
        temp << op->name << " " << reg_pairs[((ir >> 3) & 06) + 1];
        break;
    case REG2:
        temp << op->name << " " << reg_pairs[(ir >> 3) & 02];
        break;
    case ABS:
        temp << op->name << " " << std::hex << addr;
        len = 3;
        break;
    case REG:
        temp << op->name << " " << reg_names[(ir >> 3) & 07];
        break;
    case IMMR:
        temp << op->name << " " << reg_names[(ir >> 3) & 07] <<
             "," << std::hex << (addr & 0xff);
        len = 2;
        break;
    case MOV:
        temp << op->name << " " << reg_names[(ir >> 3) & 07] <<
             "," << reg_names[(ir & 07)];
        break;
    case SOPR:
        temp << op->name << " " << reg_names[(ir & 07)];
        break;
    case IMM:
        temp << op->name << " " << std::hex << (addr & 0xff);
        len = 2;
        break;

    case NUM:
        temp << op->name << " " << ((ir >> 3) & 07);
        break;
    }
    return temp.str();
}

template <cpu_model MOD>
string i8080_cpu<MOD>::dumpregs(uint8_t regs[8])
{
    int i;
    stringstream temp;

    for(i = 0; i < 8; i++) {
        if (i == M)
            continue;
        temp << reg_names[i] << "=";
        temp << hex << internal << setw(2) << setfill('0') << (unsigned int)(regs[i]) << " ";
    }
    return temp.str();
}

template <cpu_model MOD>
void emulator::i8080_cpu<MOD>::trace()
{
    string    temp;
    uint16_t  addr;
    uint8_t   ir;
    uint8_t   t;
    int       len;

    mem->read(ir, pc);
    mem->read(t, pc+1);
    addr = t;
    mem->read(t, pc+2);
    addr |= (t << 8);
    cout << dumpregs(regs) << "SP=" << hex << internal << setfill('0') << setw(4) << sp << " ";
    cout << hex << internal << setfill('0') << setw(4) << pc << " ";
    cout << hex << internal << setfill('0') << setw(2) << (unsigned int)(PSW) << " ";
    cout << disassemble(ir, addr, len) << endl;
}

template class i8080_cpu<I8080>;
template class i8080_cpu<I8085>;
}

std::map<std::string, core::CPUFactory *> core::i8080::cpu_factories;
REGISTER_CPU(i8080, I8080);
REGISTER_CPU(i8080, I8085);
