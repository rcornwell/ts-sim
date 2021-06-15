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

#include "config.h"
#if (defined(__linux) || defined(__linux__))
#ifdef HAVE_TERMIO_H
#include <termio.h>
#endif
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#endif
#if (defined(_WIN32) || defined(_WIN64))
#include <windows.h>
#include <conio.h>
#endif
#include <thread>
#include "Event.h"

namespace core
{

enum class KeyType {
    ascii, up, down, left, right, backspace, cancel, bol, eol, eof, ignore,
    del, complete, accept, kill, search_up, search_down, position
};

enum class CmdState {
    idle, quote, escape, brak, row, col
};

struct CmdKey {
    enum KeyType key;
    char       ch;
    int        row;
    int        col;
};

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

class Command_reader
{
public:
    Command_reader(void *obj, void (*function)(void *o, void *ev))
        : obj(obj), function(function) {}

    void recv_key(void *ev)
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

    virtual ~Console();


    Console(const Console&) = delete;
    Console& operator=(const Console&) = delete;

    static Console* getInstance()
    {
        static Console instance;
        return &instance;
    }
    void init();

    Event* getSendChar();

    Event* getCmd_send_char();

    void addCmd_recv_key(Command_reader *rdr);

    void addReadChar(Console_reader *rdr);

    void addWruEvent(Console_wru *wru_);

    void addAttnEvent(Console_attn *attn_);

    void shutdown();

    void show_char(void *ch);

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
#if (defined(__linux) || defined(__linux__))
    struct termios   save_termios;
    bool             term_saved = false;  // Terminal settings saved.
#endif
    bool             running = false;     // Input thread running.
    bool             use_stdin = false;   // Input via stdin rather then term.
    int              term;                // Input file handle.
    bool             mode;                // Whether to send to command or system.
    int              wru = 05;            // Wakeup character.
    int              attn = 0;            // Input attentiion.
    int              row;
    int              col;
    CmdState         cmd_state{CmdState::idle}; // Current command processing state.

#define CTRLC(x) x - '@'
#if (defined(__linux) || defined(__linux__))
    void recv_key(char ch);
#else
    void recv_key(int ch);
#endif

    static void start_reader(Console *self);

    void reader();
};


}
