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

/*
 * @file Console.h
 * @author Richard Cornwell
 * @date 28 Mar 2021
 * @brief Console input processing.
 *
 */

#pragma once

#include <termio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <thread>
#include "Event.h"

namespace core
{

class Console_reader
{
    public:
    Console_reader(void *obj, void (*function)(void *o, void *ev))
        : obj(obj), function(function) {}

    void recv_char(void *ev)
    {
        function(obj, ev);
    }

    private:
    void *obj;
    void (*function)(void *o, void *ev);
};

class Console_wru
{
    public:
    Console_wru(void *obj, void (*function)(void *o, void *ev))
        : obj(obj), function(function) {}

    void wru(void *ev)
    {
        function(obj, ev);
    }

    private:
    void *obj;
    void (*function)(void *o, void *ev);
};

class Console_attn
{
    public:
    Console_attn(void *obj, void (*function)(void *o, void *ev))
        : obj(obj), function(function) {}

    void attn(void *ev)
    {
        function(obj, ev);
    }

    private:
    void *obj;
    void (*function)(void *o, void *ev);
};

class Console
{
public:
    Console()
    {
    }

    virtual ~Console()
    {
        if (running) {
            running = false;
            // Kill running thread.
            thrd->join();
            if (term_saved) {
                int t;
                if ((t = tcsetattr(term, TCSANOW, &save_termios)) < 0) {
                    std::cerr << "Set failed " << t << " " << errno << std::endl;
            } else {
                    close(term);
                    term_saved = false;
                }
            }
        }
        delete send_char;
        delete recv_char;
    }

    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

    static Console* getInstance()
    {
        static Console instance;
        return &instance;
    }

    void init()
    {
        struct termios  buf;

        term = open("/dev/tty", O_ASYNC|O_NONBLOCK|O_RDWR);
        if (term < 0)
            return;
        if (!term_saved) {
            if (tcgetattr(term, &save_termios) < 0) /* get the original state */
                return;
        }

        if (tcgetattr(term, &buf) < 0) /* get the original state */
            return;

        buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
        /* echo off, canonical mode off, extended input
           processing off, signal chars off */

        buf.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);
        /* no SIGINT on BREAK, CR-toNL off, input parity
           check off, don't strip the 8th bit on input,
           ouput flow control off */

        buf.c_cflag &= ~(CSIZE | PARENB);
        /* clear size bits, parity checking off */

        buf.c_cflag |= CS8;
        /* set 8 bits/char */

        buf.c_oflag &= ~(OPOST);
        /* output processing off */

        buf.c_cc[VMIN] = 1;  /* 1 byte at a time */
        buf.c_cc[VTIME] = 0; /* no timer on input */

        if (tcsetattr(term, TCSANOW, &buf) >= 0) {
            term_saved = true;
        }

        // Create a thread.
        if (!running) {
            thrd = new std::thread(&Console::start_reader, this);

            // Create send character listener.
            EventCallback<Console>*callback =
                   new EventCallback<Console>(this, &Console::show_char);
            send_char = new Event();
            send_char->addListener(callback);
            // And create recieve character event handler.
            recv_char = new Event();
            EventCallback<Console>*callback2 =
                   new EventCallback<Console>(this, &Console::show_char);
            cmd_s_char = new Event();
            cmd_s_char->addListener(callback2);
            cmd_r_char = new Event();
            wru_event = new Event();
            attn_event = new Event();
       }
    };

    Event* getSendChar()
    {
        return send_char;
    };

    Event* getCmd_send_char()
    {
        return cmd_s_char;
    }

    void addCmd_recv_char(Console_reader *rdr)
    {
        EventCallback<Console_reader>*callback = new
            EventCallback<Console_reader>(rdr, &Console_reader::recv_char);
        cmd_r_char->addListener(callback);
    }

    void addReadChar(Console_reader *rdr)
    {
        EventCallback<Console_reader>*callback = new
            EventCallback<Console_reader>(rdr, &Console_reader::recv_char);
        recv_char->addListener(callback);
    };

    void addWruEvent(Console_wru *wru_)
    {
        EventCallback<Console_wru>*callback = new
            EventCallback<Console_wru>(wru_, &Console_wru::wru);
        wru_event->addListener(callback);
    };

    void addAttnEvent(Console_attn *attn_)
    {
        EventCallback<Console_attn>*callback = new
            EventCallback<Console_attn>(attn_, &Console_attn::attn);
        attn_event->addListener(callback);
    };

    void shutdown()
    {
        if (running) {
            running = false;
            // Kill running thread.
            thrd->join();
        }
        int t;
        if (term_saved) {
            if ((t = tcsetattr(term, TCSANOW, &save_termios)) < 0) {
                std::cerr << "Set failed " << t << " " << errno << std::endl;
            } else {
                term_saved = false;
                close(term);
            }
        }
    }

    void show_char(void *ch)
    {
        if (running) {
            write(term, ch, 1);
        }
    }

private:
    /**
     * @brief
     */
    std::thread      *thrd;
    Event            *send_char;
    Event            *recv_char;
    Event            *cmd_s_char;
    Event            *cmd_r_char;
    Event            *wru_event;
    Event            *attn_event;
    struct termios   save_termios;
    bool             term_saved = false;  // Terminal settings saved.
    bool             running = false;     // Input thread running.
    bool             use_stdin = false;   // Input via stdin rather then term.
    int              term;                // Input file handle.
    bool             mode;                // Whether to send to command or system.
    int              wru = 05;            // Wakeup character.
    int              attn = 0;            // Input attentiion.

    static void start_reader(Console *self)
    {
        self->reader();
    }

    void reader()
    {
    //char bell = '\007';
        running = true;
        while(running) {
            char   c;
            if (read(term, &c, 1) == 1) {
                if (c == wru) {
                    mode = !mode;
                    wru_event->notify((void *)&mode);
                } else if (attn != 0 && c == attn) {
                    attn_event->notify((void *)&c);
                } else if (mode) {
                    cmd_r_char->notify((void *)&c);
                } else {
                    recv_char->notify((void *)&c);
                }
            }
        }
        return;
    }


};


}
