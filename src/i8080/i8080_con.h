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

#include "Event.h"
#include "Console.h"
#include "Device.h"

#define DATA_PORT      0
#define STATUS_PORT    1
#define MODE_PORT      2
#define CMD_PORT       3

// Status port bit definitions.
#define TxRDY          0x01    // set indicates transmitter ready.
#define RxRDY          0x02    // set indicates character ready.
#define TxEMT          0x04    // Indicates DCD or DSR changed.
#define RxPE           0x08    // Parity error.
#define RxOVER         0x10    // Indicates character no read.
#define RxFE           0x20    // Indicates there was a framing error.
#define DCD            0x40    // State of DCD line.
#define DSR            0x80    // State of DSR line.

// Mode byte 1.
#define MODE_MASK      0x03    // Mode bits.
#define MODE_SYNC      0x00    // Sychronous operation.
#define MODE_ASYNC1x   0x01    // Asynchronous 1x clock rate.
#define MODE_ASYNC16x  0x02    // Asynchronous 16x clock rate.
#define MODE_ASYNC64x  0x03    // Asynchronous 64x clock rate.
#define CHAR_LENGTH    0x0c    // Character length mask.
#define CHAR_5BIT      0x00    // Character length is 5 bits.
#define CHAR_6BIT      0x04    // Character length is 6 bits.
#define CHAR_7BIT      0x08    // Character length is 7 bits.
#define CHAR_8BIT      0x0c    // Character length is 8 bits.
#define PARITY_ENABLE  0x10    // Enable parity support.
#define PARITY_EVEN    0x20    // Set parity to Odd.
#define SYNC_TRANS     0x40    // Synchronous transpanerant control.
#define SYNC_SINGLE    0x80    // Single synch char.
#define ASYNC_MASK     0xc0    // Stop bit mask
#define STOP_1BIT      0x40    // One stop bit.
#define STOP_HBIT      0x80    // 1.5 stop bits.
#define STOP_2BIT      0xc0    // 2 stop bits.

// Mode byte 2.
#define BAUD_RATE      0x0f    // Baud rate mask.
#define RECV_CLOCK     0x10    // Internal recieve clock.
#define TRAN_CLOCK     0x20    // Internal transmitt clock.

// Command byte.
#define TRAN_ENABLE    0x01    // Enable transmitter.
#define DTR            0x02    // State of DTR line.
#define RECV_ENABLE    0x04    // Enable reciever.
#define BREAK          0x08    // Send break.
#define RESET          0x10    // Clear error flags in status register.
#define RTS            0x20    // State of RTS line.
#define ASYNC_MASK     0xc0    // Mask for async control.
#define ASYNC_ECHO     0x40    // Automatic echo mode.
#define ASYNC_LOCAL    0x80    // Local loopback
#define ASYNC_REMOTE   0xc0    // Remote loopback.


// Local Loopback mode.
//   transmit connected to reciever.
//   dtr -> dcd
//   rts -> cts
//
// Remote loopback mode.
//   reciever connected to transmitter.
//   No data recieved.




namespace emulator
{

class i8080_2651 : public Device<uint8_t>
{
public:
    i8080_2651() : Device()
    {
    }

    i8080_2651(const std::string &name) : Device(name)
    {
    }

    virtual ~i8080_2651()
    {
    }

    virtual size_t getSize() const override
    {
        return 4;
    }

    virtual void init() {
        con = core::Console::getInstance();
        con->init();
        send_char = con->getSendChar();
        core::Console_reader *rdr = new core::Console_reader(this, &recv_ch);
        con->addReadChar(rdr);
    }

    virtual void shutdown()
    {
        con->shutdown();
    }

    //virtual void start() {}
    virtual void reset()
    {
        mode_ptr_ = false;
        status_ = 0;
        cmd_ = 0;
        mode1_ = 0;
        mode2_ = 0;
    }
    //virtual void stop() {}
    //virtual void step() {}
    //virtual void run() {}
    //virtual void examine() {}
    //virtual void deposit() {}

    virtual bool input(uint8_t &val, size_t port) override
    {
        switch ((int)(port - addr_) & 0x3) {
        case DATA_PORT:
            // Return read character.
            val = recv_buff;
            recv_full = false;
            break;

        case STATUS_PORT:
            val = status_;
            if (recv_full)
                val |= RxRDY;
            if (over_run)
                val |= RxOVER;
            break;

        case MODE_PORT:
            val = (mode_ptr_) ? mode2_ : mode1_;
            mode_ptr_ = !mode_ptr_;
            break;

        case CMD_PORT:
            val = cmd_;
            break;

        default:
            val = 0;
            return false;
        }
        return true;
    }

    virtual bool output(uint8_t val, size_t port) override
    {
        char ch;
        switch ((int)(port - addr_) & 0x3) {
        case DATA_PORT:
            // transmit character.
            //std::cout << val << std::flush;
            ch = (char)val;
            send_char->notify((void *)&ch);
            break;
        case STATUS_PORT:
            // Write syn1/syn2/dle characters.
            break;
        case MODE_PORT:
            if (mode_ptr_)
                mode2_ = val;
            else
                mode1_ = val;
            break;

        case CMD_PORT:
            cmd_ = val;
            mode_ptr_ = false;
            if (cmd_ & RESET) {
                status_ &= (~RxPE|RxOVER|RxFE);
                over_run = false;
            }
            if (cmd_ & TRAN_ENABLE) {
                // Enable transmitter.
                status_ |= TxRDY;
            }

            break;
        default:
            val = 0;
            return false;
        }
        return true;
    }

    virtual
    core::ConfigOptionParser options()
    {
        core::ConfigOptionParser option("Device Options");
        return option;
    }

    void setCPU(shared_ptr<CPU<uint8_t>> cpu_)
    {
        cpu = cpu_;
    }
    shared_ptr<CPU<uint8_t>> cpu;

    static void recv_ch(void *obj, void *ev)
    {
        char ch = *((char *)ev);
        i8080_2651 *o = (i8080_2651 *)obj;
        if (ch == 03) {
            o->cpu->running = false;
            return;
        }
        o->recv_buff = ch;
        if (o->recv_full)
            o->over_run = true;
        o->recv_full = true;
    }

    private:
    core::Console       *con;
    core::Event         *send_char;
    core::Console_reader *recv_char;
    uint8_t     mode1_;
    uint8_t     mode2_;
    bool        mode_ptr_ = false;
    uint8_t     cmd_;
    uint8_t     status_;
    uint8_t     recv_buff;
    bool        recv_full;
    bool        over_run;
};

}

REGISTER_DEVICE(i8080, 2651)
