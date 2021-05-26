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


#define CC(f,t,b,p,m) \
    CCX(f##cc,b,0,f##nz,ZERO, ==, t, m) \
    CCX(f##cc,b,1,f##z,ZERO, !=, t, m) \
    CCX(f##cc,b,2,f##nc,CARRY, ==, t, m) \
    CCX(f##cc,b,3,f##c,CARRY, !=, t, m) \
    CCX(f##cc,b,4,f##pe,PAR, ==, t, m) \
    CCX(f##cc,b,5,f##po,PAR, !=, t, m) \
    CCX(f##cc,b,6,f##p, SIGN, ==, t, m) \
    CCX(f##cc,b,7,f##m, SIGN, !=, t, m)
    
INSN(nop,  OPR,  0000, I8080)
INSN(lxi,  LXI,  0001, I8080)
INSN(dad,  REGX, 0011, I8080)
INSN(stax, REG2, 0002, I8080)
INSN(ldax, REG2, 0012, I8080)
INSN(shld, ABS,  0042, I8080)
INSN(lhld, ABS,  0052, I8080)
INSN(sta,  ABS,  0062, I8080)
INSN(lda,  ABS,  0072, I8080)
INSN(inr,  REG,  0004, I8080)
INSN(dcr,  REG,  0005, I8080)
INSN(inx,  REGX, 0003, I8080)
INSN(dcx,  REGX, 0013, I8080)
INSN(mvi,  IMMR, 0006, I8080)
INSN(rlc,  OPR,  0007, I8080)
INSN(rrc,  OPR,  0017, I8080)
INSN(ral,  OPR,  0027, I8080)
INSN(rar,  OPR,  0037, I8080)
INSN(daa,  OPR,  0047, I8080)
INSN(cma,  OPR,  0057, I8080)
INSN(stc,  OPR,  0067, I8080)
INSN(cmc,  OPR,  0077, I8080)
INSN(mov,  MOV,  0100, I8080)
INSN(hlt,  OPR,  0166, I8080)
INSN(add,  SOPR, 0200, I8080)
INSN(adc,  SOPR, 0210, I8080)
INSN(sub,  SOPR, 0220, I8080)
INSN(sbb,  SOPR, 0230, I8080)
INSN(ana,  SOPR, 0240, I8080)
INSN(xra,  SOPR, 0250, I8080)
INSN(ora,  SOPR, 0260, I8080)
INSN(cmp,  SOPR, 0270, I8080)
INSN(r,    CCR,  0300, I8080)
INSN(j,    CCJ,  0302, I8080)
INSN(c,    CCC,  0304, I8080)
INSN(pop,  REGP, 0301, I8080)
INSN(ret,  OPR,  0311, I8080)
INSN(pchl, OPR,  0351, I8080)
INSN(sphl, OPR,  0371, I8080)
INSN(adi,  IMM,  0306, I8080)
INSN(aci,  IMM,  0316, I8080)
INSN(sui,  IMM,  0326, I8080)
INSN(sbi,  IMM,  0336, I8080)
INSN(ani,  IMM,  0346, I8080)
INSN(xri,  IMM,  0356, I8080)
INSN(ori,  IMM,  0366, I8080)
INSN(cpi,  IMM,  0376, I8080)
INSN(push, REGP, 0305, I8080)
INSN(call, ABS,  0315, I8080)
INSN(jmp,  ABS,  0303, I8080)
INSN(out,  IMM,  0323, I8080)
INSN(in,   IMM,  0333, I8080)
INSN(di,   OPR,  0363, I8080)
INSN(ei,   OPR,  0373, I8080)
INSN(xcht, OPR,  0343, I8080)
INSN(xchg, OPR,  0353, I8080)
INSN(rst,  RST,  0307, I8080)
INSN(rim,  OPR,  0040, I8085)
INSN(sim,  OPR,  0060, I8085)
INSN(dsub, OPR,  0010, I8085)
INSN(arhl, OPR,  0020, I8085)
INSN(rdel, OPR,  0030, I8085)
INSN(ldhi, IMM,  0050, I8085)
INSN(ldsi, IMM,  0070, I8085)
INSN(rstv, OPR,  0313, I8085)
INSN(shlx, OPR,  0331, I8085)
INSN(jnx5, ABS,  0335, I8085)
INSN(lhlx, OPR,  0355, I8085)
INSN(jx5,  ABS,  0375, I8085)
