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

#pragma once

#include <string>
#include <sstream>
#include <stdint.h>
#include "CPU.h"
#include "Memory.h"
#include "ConfigOption.h"
#include "i8080_system.h"


namespace emulator
{


using namespace std;
#define SIGN  0x80
#define ZERO  0x40
#define XFLG  0x20
#define AC    0x10
#define PAR   0x04
#define VFLG  0x02
#define CARRY 0x01

enum cpu_model {
    I8080, I8085, Z80
};


enum reg_name {
    B, C, D, E, H, L, M, A
};

enum reg_pair {
    BC, DE, HL, SP, PW
};


template <enum cpu_model MOD>
class i8080_cpu : public CPU<uint8_t>
{
public:
    i8080_cpu() 
    {
    }
    virtual ~i8080_cpu()
    {
    }

    auto getType() const -> string
    {
        if constexpr (MOD == I8080) return "I8080";
        if constexpr (MOD == I8085) return "I8085";
    }


    uint16_t  sp;
    bool      ie;

    uint8_t   regs[8];
    uint8_t   PSW;

    int       cycle_time;
    int       page_size;

    virtual
    core::ConfigOptionParser options() override {
        core::ConfigOptionParser option("CPU options");
        auto page_opt = option.add<core::ConfigValue<int>>("pagesize", "address spacing", 0, &page_size);
        return option;
    }
    
#define Tc 250
    int   ins_time[256] = {
        /*   0     1     2     3     4     5     6     7 */
        4*Tc, 10*Tc,  7*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  4*Tc,    /* 00x */
        4*Tc, 10*Tc,  7*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  4*Tc,    /* 01x */
        4*Tc, 10*Tc,  7*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  4*Tc,    /* 02x */
        4*Tc, 10*Tc,  7*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  4*Tc,    /* 03x */
        4*Tc, 16*Tc,  7*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  4*Tc,    /* 04x */
        4*Tc, 16*Tc,  7*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  4*Tc,    /* 05x */
        4*Tc, 16*Tc,  7*Tc,  5*Tc, 10*Tc, 10*Tc, 10*Tc,  4*Tc,    /* 06x */
        4*Tc, 16*Tc,  7*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  4*Tc,    /* 07x */

        5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  5*Tc,    /* 10x */
        5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  5*Tc,    /* 11x */
        5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  5*Tc,    /* 12x */
        5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  5*Tc,    /* 13x */
        5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  5*Tc,    /* 14x */
        5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  5*Tc,    /* 15x */
        7*Tc,  7*Tc,  7*Tc,  7*Tc,  7*Tc,  7*Tc, 10*Tc,  7*Tc,    /* 16x */
        5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  5*Tc,  7*Tc,  5*Tc,    /* 17x */

        4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,    /* 20x */
        4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,    /* 21x */
        4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,    /* 22x */
        4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,    /* 23x */
        4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,    /* 24x */
        4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,    /* 25x */
        7*Tc,  7*Tc,  7*Tc,  7*Tc,  7*Tc,  4*Tc,  7*Tc,  7*Tc,    /* 26x */
        4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,  4*Tc,    /* 27x */

        5*Tc, 10*Tc, 10*Tc, 10*Tc, 11*Tc, 11*Tc,  7*Tc, 11*Tc,    /* 30x */
        5*Tc, 10*Tc, 10*Tc,  4*Tc, 11*Tc, 17*Tc,  7*Tc, 11*Tc,    /* 31x */
        5*Tc, 10*Tc, 10*Tc, 10*Tc, 11*Tc, 11*Tc,  7*Tc, 11*Tc,    /* 32x */
        5*Tc, 10*Tc, 10*Tc, 10*Tc, 11*Tc,  4*Tc,  7*Tc, 11*Tc,    /* 33x */
        5*Tc, 10*Tc, 10*Tc, 18*Tc, 11*Tc, 11*Tc,  7*Tc, 11*Tc,    /* 34x */
        5*Tc, 10*Tc, 10*Tc,  4*Tc, 11*Tc,  4*Tc,  7*Tc, 11*Tc,    /* 35x */
        5*Tc,  5*Tc, 10*Tc,  4*Tc, 11*Tc, 11*Tc,  7*Tc, 11*Tc,    /* 36x */
        5*Tc,  5*Tc, 10*Tc,  4*Tc, 11*Tc,  4*Tc,  7*Tc, 11*Tc,    /* 37x */
    };


    /**
     * @brief Fetches the contents of memory location pointed to by HL.
     * @return Value of memory location.
     */
    inline uint8_t fetch_mem()
    {
        uint8_t t;
        mem->read(t, regpair<HL>());
        return t;
    }

    /**
     * @brief Fetch the register specified by the parameter R.
     *
     * This function is templated to allow for inline generation of the correct
     * function based on definitions. The code should be expanded to just fetch
     * the value or the contects of memory.
     * @return Value of the register/memory.
     */
    template <reg_name R>
    constexpr inline uint8_t fetch_reg()
    {
        uint8_t t;
        if constexpr (R != M)
            t = regs[(int)R];
        else
            t = fetch_mem();
        return t;
    }

    /**
     * @brief Sets the register pointed to by the parameter R.
     * @param value Value to set register to.
     * @return Void.
     */
    template <reg_name R>
    constexpr inline void set_reg(uint8_t value)
    {
        if constexpr (R != M)
            regs[(int)R] = value;
        else
            mem->write(value, regpair<HL>());
    }

    /**
     * @brief Fetch the register pair pointed to by the parameter RP.
     *
     * The register is returned as a 16 bit value.
     * @return Value of register pair.
     */
    template <reg_pair RP>
    inline uint16_t regpair()
    {
        uint16_t t;
        if constexpr (RP == BC) {
            t = ((uint16_t)(regs[B]) << 8) |
                ((uint16_t)(regs[C]));
        } else if constexpr (RP == DE) {
            t = ((uint16_t)(regs[D]) << 8) |
                ((uint16_t)(regs[E]));
        } else if constexpr (RP == HL) {
            t = ((uint16_t)(regs[H]) << 8) |
                ((uint16_t)(regs[L]));
        } else if constexpr (RP == SP) {
            t = sp;
        } else if constexpr (RP == PW) {
            t = (uint16_t)(PSW) |
                ((uint16_t)(regs[A])<<8);
        }
        return t;
    }

    /**
     * @brief Sets the register pair specified by the parameter RP
     * @param value Value to set register pair to.
     * @return Void
     */
    template <int RP>
    constexpr inline void setregpair(uint16_t value)
    {
        if constexpr (RP == BC) {
            regs[B] = (value >> 8) & 0xff;
            regs[C] = value & 0xff;
        } else if constexpr (RP == DE) {
            regs[D] = (value >> 8) & 0xff;
            regs[E] = value & 0xff;
        } else if constexpr (RP == HL) {
            regs[H] = (value >> 8) & 0xff;
            regs[L] = value & 0xff;
        } else if constexpr (RP == SP) {
            sp = value;
        } else if constexpr (RP == PW) {
            if constexpr  (MOD == I8080) {
                PSW = ((value) & (SIGN|ZERO|AC|PAR|CARRY)) | VFLG;
            }
            if constexpr  (MOD == I8085) {
                PSW = (value) & (SIGN|ZERO|XFLG|AC|PAR|VFLG|CARRY);
            }
            regs[A] = (value >> 8) & 0xff;
        }
    }

    /**
     * @brief Fetch the next byte pointed to by the program counter.
     *
     * After the fetch the program counter is incremented by 1.
     *
     * @return Next byte pointed to by program counter.
     */
    inline uint8_t fetch()
    {
        uint8_t temp;

        if (mem->read(temp, pc)) {
            pc ++;
            pc &= 0xffff;
            return temp;
        }
        return 0166;
    }

    /**
     * @brief Return the address at the program counter.
     *
     * The next two bytes of program memory are read and converted to a
     * 16 bit value.
     *
     * @return Address value of next two bytes.
     */
    inline uint16_t fetch_addr()
    {
        uint16_t value;
        uint8_t  temp;
        if (mem->read(temp, pc)) {
            pc ++;
            pc &= 0xffff;
            value = temp;
            if (mem->read(temp, pc)) {
                pc++;
                pc &= 0xffff;
                value |= ((uint16_t)temp) << 8;
                return value;
            }
        }
        return 0;
    }

    /**
     * @brief Returns the address word from the two bytes at addr.
     * @param addr Address to fetch next two bytes from.
     * @return Next two bytes as address.
     */
    inline uint16_t fetch_double(uint16_t addr)
    {
        uint16_t value;
        uint8_t  temp;

        (void)(mem->read(temp, addr));
        addr ++;
        addr &= 0xffff;
        value = temp;
        (void)(mem->read(temp, addr));
        value |= ((uint16_t)temp) << 8;
        return value;
    }

    /**
     * @brief Save the address (value) into the two bytes pointed to by
     * addr.
     * @param value Value to save.
     * @param addr Location to save value.
     */
    inline void store_double(uint16_t value, uint16_t addr)
    {
        uint8_t temp = (value & 0xff);
        mem->write(temp, addr);
        addr ++;
        addr &= 0xffff;
        temp = (value >> 8) & 0xff;
        mem->write(temp, addr);
    }

    /**
     * @brief Compute the parity of the argument
     * @param v Byte to compute parity of
     * @returns PAR flag or 0.
     */
    uint8_t flag_gen(uint8_t v);

    /**
     * @brief Push value as two byte value onto stack.
     * @param value to push
     */
    inline void push(uint16_t value)
    {
        sp -= 2;
        store_double(value, sp);
    }

    /**
     * @brief Pop next two byte value off stack.
     * @return Value popped.
     */
    inline uint16_t pop()
    {
        uint16_t  value;
        value = fetch_double(sp);
        sp += 2;
        return value;
    }

    /**
     * @brief Convert INSN macro into definitions for instructions.
     *
     * INSN defines an instruction name, the type of the instruction
     * and the base of the opcode. All opcodes are defined as o_name().
     * Depending on the function some will expand based on the register
     * or the register pair of there argument.
     **/
#define OPR(f,b,m)   void o_##f();
#define ABS(f,b,m)   OPR(f,b,m)
#define REG(f,b,m)   template <reg_name R>void o_##f();
#define REGX(f,b,m)  template <reg_pair RP>void o_##f();
#define REGP(f,b,m)  REGX(f,b,m)
#define LXI(f,b,m)   REGX(f,b,m)
#define IMMR(f,b,m)
#define IMM(f,b,m)   void o_##f();
#define REG2(f,b,m)  REGX(f,b,m)
#define MOV(f,b,m)
#define SOPR(f,a,m)  void o_##f(uint8_t data);
#define CCX(f)       void o_##f##cc(int c);
#define CCR(f,b,m)   CCX(f)
#define CCJ(f,b,m)   CCX(f)
#define CCC(f,b,m)   CCX(f)
#define RST(f,b,m)
#define INSN(name, type, base, mod) type(name, base, mod)

#include "../i8080/i8080_insn.h"

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

    void decode(uint8_t op);

    virtual void init() override
    {
        Memory<uint8_t> *memctl{new MemArray<uint8_t>((size_t)(64*1024), (size_t)page_size)};
        SetMem(memctl);
        SetIO(new IO_map<uint8_t>(256u));
    };

    void shutdown() {};

    void start()
    {
        running = true;
    };

    void reset()
    {
        running = false;
        pc = 0;
        PSW = 2;
        ie = false;
    };

    void stop()
    {
        running = false;
    };

    void trace()
    ;

    uint64_t step();

    void run() {};

    string disassemble(uint8_t ir, uint16_t addr, int &len);

    string dumpregs(uint8_t regs[8]);
};

};
