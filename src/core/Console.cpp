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
 * @file Console.cpp
 * @author Richard Cornwell
 * @date 28 Mar 2021
 * @brief Console input processing.
 *
 */

#include "config.h"
#include <iostream>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
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
#include "Console.h"

namespace core
{


Console::~Console()
{
    if (running) {
        running = false;
        // Kill running thread.
        thrd->join();
#if (defined(__linux) || defined(__linux__))
        if (term_saved) {
            int t;
            if ((t = tcsetattr(term, TCSANOW, &save_termios)) < 0) {
                std::cerr << "Set failed " << t << " " << errno << std::endl;
            } else {
                close(term);
                term_saved = false;
            }
        }
#endif
    }
    delete send_char;
    delete recv_char;
}


void Console::init()
{
#if (defined(__linux) || defined(__linux__))
    struct termios  buf;

    if (term_saved)
        return;
    term = open("/dev/tty", O_ASYNC|O_NONBLOCK|O_RDWR);
    if (term < 0)
        return;
    if (tcgetattr(term, &save_termios) < 0) /* get the original state */
        return;

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
#endif

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

Event* Console::getSendChar()
{
    return send_char;
};

Event* Console::getCmd_send_char()
{
    return cmd_s_char;
}

void Console::addCmd_recv_key(Command_reader *rdr)
{
    EventCallback<Command_reader>*callback = new
    EventCallback<Command_reader>(rdr, &Command_reader::recv_key);
    cmd_r_char->addListener(callback);
}

void Console::addReadChar(Console_reader *rdr)
{
    EventCallback<Console_reader>*callback = new
    EventCallback<Console_reader>(rdr, &Console_reader::recv_char);
    recv_char->addListener(callback);
};

void Console::addWruEvent(Console_wru *wru_)
{
    EventCallback<Console_wru>*callback = new
    EventCallback<Console_wru>(wru_, &Console_wru::wru);
    wru_event->addListener(callback);
};

void Console::addAttnEvent(Console_attn *attn_)
{
    EventCallback<Console_attn>*callback = new
    EventCallback<Console_attn>(attn_, &Console_attn::attn);
    attn_event->addListener(callback);
};

void Console::shutdown()
{
    if (running) {
        running = false;
        // Kill running thread.
        thrd->join();
    }
#if (defined(__linux) || defined(__linux__))
    if (term_saved) {
        int t;
        if ((t = tcsetattr(term, TCSANOW, &save_termios)) < 0) {
            std::cerr << "Set failed " << t << " " << errno << std::endl;
        } else {
            term_saved = false;
            close(term);
        }
    }
#endif
}

void Console::show_char(void *ch)
{
    if (running) {
#if (defined(_WIN32) || defined(_WIN64))
        std::cout << ch << std::flush;
#else
        while(1) {
            int r = write(term, ch, 1);
            if (r == 1 || (errno != EAGAIN && errno != EINTR))
                return;
        }
#endif
    }
}

#define CTRLC(x) x - '@'
#if (defined(__linux) || defined(__linux__))
void Console::recv_key(char ch)
{
    CmdKey    key;
    switch(cmd_state) {
    case CmdState::idle:
    default:
        key.ch = ch;
        // Normal processing functions.
        switch (ch) {
        case CTRLC('A'):
            key.key = KeyType::bol;
            break;
        case CTRLC('B'):
            key.key = KeyType::left;
            break;
        case CTRLC('C'):
            key.key = KeyType::cancel;
            break;
        case CTRLC('D'):
            key.key = KeyType::del;
            break;
        case CTRLC('E'):
            key.key = KeyType::eol;
            break;
        case CTRLC('F'):
            key.key = KeyType::right;
            break;
        case CTRLC('H'):
        case '\177':
            key.key = KeyType::backspace;
            break;
        case CTRLC('I'):
            key.key = KeyType::complete;
            break;
        case CTRLC('J'):
        case CTRLC('M'):
            key.key = KeyType::accept;
            break;
        case CTRLC('K'):
            key.key = KeyType::kill;
            break;
        case CTRLC('N'):
            key.key = KeyType::down;
            break;
        case CTRLC('P'):
            key.key = KeyType::up;
            break;
        case CTRLC('V'):
            cmd_state = CmdState::quote;
            break;
        case CTRLC('S'):
            key.key = KeyType::search_up;
            break;
        case CTRLC('R'):
            key.key = KeyType::search_down;
            break;
        case '\033':
            cmd_state = CmdState::escape;
            return;
        default:
            key.key = KeyType::ascii;
            break;
        }
        break;
    case CmdState::quote:
        cmd_state = CmdState::idle;
        key.key = KeyType::ascii;
        break;
    case CmdState::escape:
        if (ch == '[') {
            cmd_state = CmdState::brak;
            return;
        } else {
            // Ignore sequence.
            cmd_state = CmdState::idle;
            return;
        }
        break;
    case CmdState::brak:
        // Process escape [ sequences.
        cmd_state = CmdState::idle;
        switch (ch) {
        case 'A':
            key.key = KeyType::up;
            break;
        case 'B':
            key.key = KeyType::down;
            break;
        case 'C':
            key.key = KeyType::right;
            break;
        case 'D':
            key.key = KeyType::left;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            row = ch - '0';
            cmd_state = CmdState::row;
            return;
        default:
            cmd_state = CmdState::idle;
            return;
        }
        break;
    case CmdState::row:
        switch(ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            row = (row * 10) + (ch - '0');
            return;
        case ';':
            col = 0;
            cmd_state = CmdState::col;
            return;
        default:
            cmd_state = CmdState::idle;
            return;
        }
        break;
    case CmdState::col:
        switch(ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            col = (col * 10) + (ch - '0');
            return;
        case 'R':
            key.key = KeyType::position;
            key.row = row;
            key.col = col;
            cmd_state = CmdState::idle;
            break;
        default:
            cmd_state = CmdState::idle;
            return;
        }
        break;
    }
    cmd_r_char->notify((void *)&key);
}
#else
void Console::recv_key(int ch)
{
    CmdKey    key;
    switch(cmd_state) {
    case CmdState::idle:
    default:
        key.ch = ch;
        // Normal processing functions.
        switch (ch) {
        case CTRLC('A'):
            key.key = KeyType::bol;
            break;
        case CTRLC('B'):
            key.key = KeyType::left;
            break;
        case CTRLC('C'):
            key.key = KeyType::cancel;
            break;
        case CTRLC('D'):
            key.key = KeyType::del;
            break;
        case CTRLC('E'):
            key.key = KeyType::eol;
            break;
        case CTRLC('F'):
            key.key = KeyType::right;
            break;
        case CTRLC('H'):
            key.key = KeyType::backspace;
            break;
        case CTRLC('I'):
            key.key = KeyType::complete;
            break;
        case CTRLC('J'):
        case CTRLC('M'):
            key.key = KeyType::accept;
            break;
        case CTRLC('K'):
            key.key = KeyType::kill;
            break;
        case CTRLC('N'):
            key.key = KeyType::down;
            break;
        case CTRLC('P'):
            key.key = KeyType::up;
            break;
        case CTRLC('V'):
            cmd_state = CmdState::quote;
            break;
        case CTRLC('S'):
            key.key = KeyType::search_up;
            break;
        case CTRLC('R'):
            key.key = KeyType::search_down;
            break;
        case '\033':
            cmd_state = CmdState::escape;
            return;
        default:
            key.key = KeyType::ascii;
            break;
        }
        break;
    case CmdState::quote:
        cmd_state = CmdState::idle;
        key.key = KeyType::ascii;
        break;
    case CmdState::escape:
        if (ch == '[') {
            cmd_state = CmdState::brak;
            return;
        } else {
            // Ignore sequence.
            cmd_state = CmdState::idle;
            return;
        }
        break;
    case CmdState::brak:
        // Process escape [ sequences.
        cmd_state = CmdState::idle;
        switch (ch) {
        case 'A':
            key.key = KeyType::up;
            break;
        case 'B':
            key.key = KeyType::down;
            break;
        case 'C':
            key.key = KeyType::right;
            break;
        case 'D':
            key.key = KeyType::left;
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            row = ch - '0';
            cmd_state = CmdState::row;
            return;
        default:
            cmd_state = CmdState::idle;
            return;
        }
        break;
    case CmdState::row:
        switch(ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            row = (row * 10) + (ch - '0');
            return;
        case ';':
            col = 0;
            cmd_state = CmdState::col;
            return;
        default:
            cmd_state = CmdState::idle;
            return;
        }
        break;
    case CmdState::col:
        switch(ch) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            col = (col * 10) + (ch - '0');
            return;
        case 'R':
            key.key = KeyType::position;
            key.row = row;
            key.col = col;
            cmd_state = CmdState::idle;
            break;
        default:
            cmd_state = CmdState::idle;
            return;
        }
        break;
    }
}
cmd_r_char->notify((void *)&key);
}
#endif

void Console::start_reader(Console *self)
{
    self->reader();
}

void Console::reader()
{
    //char bell = '\007';
    running = true;
    while(running) {
#if (defined(_WIN32) || defined(_WIN64))
        int    c = _getch();
#else
        char   c;
        if (read(term, &c, 1) == 1) {
#endif
        // If we are in quote state accept next char unchecked.
        if (cmd_state == CmdState::quote) {
            recv_key(c);
            cmd_state = CmdState::idle;
        } else if (c == wru) {
            mode = !mode;
            wru_event->notify((void *)&mode);
        } else if (attn != 0 && c == attn) {
            attn_event->notify((void *)&c);
        } else if (mode) {
            recv_key(c);
        } else {
            recv_char->notify((void *)&c);
        }
#if !(defined(_WIN32) || defined(_WIN64))
    }
#endif
}
return;
}

}
