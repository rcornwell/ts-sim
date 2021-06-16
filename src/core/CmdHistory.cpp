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
#include <string>
#include <iostream>
#include "Command.h"
#include "CmdHistory.h"

namespace core
{

static void key_ev(void *obj, void *ev)
{
    CmdKey *key = ((CmdKey *)ev);
    CmdHistory *o = (CmdHistory *)obj;
    o->recv_key(key);
}

static void wru_ev(void *obj, void *ev)
{
    bool mode = *((bool *)ev);
    CmdHistory *o = (CmdHistory *)obj;
    o->wru(mode);
}

void CmdHistory::init()
{
    con = core::Console::getInstance();
    con->init();
    send_char = con->getCmd_send_char();
    Command_reader *rdr = new Command_reader(this, &key_ev);
    con->addCmd_recv_key(rdr);
    Console_wru *wru = new Console_wru(this, &wru_ev);
    con->addWruEvent(wru);
}

void CmdHistory::send(const std::string & str)
{
    for(const char& c : str) {
        //     std::cerr << "Sending: " << c << std::endl;
        send_char->notify((void *)&c);
        pos++;
    }
}

void CmdHistory::send_esc(const std::string & str)
{
    for(const char& c : str) {
        send_char->notify((void *)&c);
    }
}

void CmdHistory::leftChar()
{
    // If at beginning of line, stop.
    if (buf_ptr == 0)
        return;
    // Copy character to end of buffer.
    buffer[--buf_end] = buffer[--buf_ptr];
    // Back up cursor by 1.
    send_esc("\033[D");
    pos--;
}

void CmdHistory::rightChar()
{
    // If nothing at end, don't do anything.
    if (buf_end == (sizeof(buffer)-1))
        return;
    // Copy end character to front of buffer.
    buffer[buf_ptr++] = buffer[buf_end++];
    // Send the character.
    send_char->notify((void *)&buffer[buf_ptr - 1]);
    pos++;
}

void CmdHistory::moveBol()
{
    // Move backwards until we hit beginning of line.
    while (buf_ptr != 0)
        leftChar();
}

void CmdHistory::moveEol()
{
    // Move right until we hit end of line or end of buffer.
    while (buf_end < (sizeof(buffer)-1)) {
        char ch;
        ch = buffer[buf_end];
        if (ch == '\r')
            return;
        rightChar();
    }
}

void CmdHistory::backSpace()
{
    // If at beggining of line, don't do anything.
    if (buf_ptr == 0)
        return;
    if (buf_end < (sizeof(buffer)-1)) {
        send_esc("\033[0K"); // Erase rest of line.
    }
    send_esc("\033[D \033[D"); // Clear last character.
    buf_ptr--;
    pos--;
    // Refresh rest of line.
    int count = 0;
    for (size_t i = buf_end; i < (sizeof(buffer)-1); i++) {
        char c;
        c = buffer[i];
        if (c == '\r')      // Stop at end of line
            break;
        send_char->notify((void *)&c);  // Echo remainder of line.
        count++;
    }
    // Move cursor back to input spot.
    if (count > 0) {
        std::string back = "\033[" + std::to_string(count) + "D";
        send_esc(back);
    }
}

void CmdHistory::cancelLine()
{
    // Move to beginning of line.
    moveBol();
    // Clear to end of line.
    deleteEol();
}

void CmdHistory::deleteNext()
{
    // If at end of line nothing to do.
    if (buf_end == (sizeof(buffer)-1))
        return;
    char ch = buffer[buf_end];
    // If at end of line nothing to do.
    if (ch == '\r')
        return;
    // Clear to end of line.
    send_esc("\033[0K");
    // Delete next character.
    buf_end++;
    // Refresh the rest of line.
    int count = 0;
    for (size_t i = buf_end; i < (sizeof(buffer)-1); i++) {
        char c;
        c = buffer[i];
        if (c == '\r')          // Stop at end of line
            break;
        send_char->notify((void *)&c);  // Echo remainder of line.
        count++;
    }
    // Move cursor back to input spot.
    if (count > 0) {
        std::string back = "\033[" + std::to_string(count) + "D";
        send_esc(back);
    }
}

void CmdHistory::deleteEol()
{
    // Clear to end of line.
    if (buf_end < (sizeof(buffer)-1)) {
        send_esc("\033[0K");
    }
    // Advance pointer until end of line.
    while (buf_end < (sizeof(buffer)-1)) {
        char ch = buffer[buf_end];
        if (ch == '\r')
            break;
        buf_end++;
    }
}

void CmdHistory::insertChar(char ch)
{
    if (buf_end < (sizeof(buffer)-1)) {
        send_esc("\033[0K");  // Erase rest of line.
    }
    buffer[buf_ptr++] = ch;
    send_char->notify((void *)&ch);  // Send character.
    int count = 0;
    // Refresh rest of line
    for (size_t i = buf_end; i < (sizeof(buffer)-1); i++) {
        char c;
        c = buffer[i];
        if (c == '\r')          // Stop at end of line
            break;
        send_char->notify((void *)&c);  // Echo remainder of line.
        count++;
    }
    // Move cursor back to input spot.
    if (count > 0) {
        std::string back = "\033[" + std::to_string(count) + "D";
        send_esc(back);
    }
}

void CmdHistory::acceptLine()
{
    std::string line;

    // Copy both halves into a string.
    for (size_t i = 0; i < buf_ptr; i++)
        line += buffer[i];
    for (size_t i = buf_end; i < (sizeof(buffer)-1); i++)
        line += buffer[i];
    // If last character is a \ then continue reading input.
    if (line.back() == '\\') {
        std::cerr << "\r\nMultiLine: " + line << std::endl;
        multiline = true;
        row ++;
        return;
    }
    std::cerr << "\r\nComand string: " + line << std::endl;
    history.push_back(line);
    hist_pos = history.size();
}

void CmdHistory::moveUp()
{
    if (multiline) {
        // Handle multiline history.
    }
    // Don't do anything when at top.
    if (hist_pos == 0)
        return;
    // Back up to previous entry.
    hist_pos--;
    // Clear the line
    send_esc("\r\033[0K");
    // Save current position.
    size_t cur_pos = buf_ptr;
    pos = 0;
    // Print out prompt.
    send(prompt);
    // Grab last history line.
    std::string line = history.at(hist_pos);
    size_t len = line.size();
    buf_ptr = 0;
    buf_end = sizeof(buffer)-1;
    // Copy over to buffer.
    for(size_t i=0; i < len; i++) {
        buffer[buf_ptr] = line[i];
        send_char->notify((void *)&buffer[buf_ptr]);
        buf_ptr++;
        pos++;
    }
    // Put cursor in about same place.
    while(buf_ptr > cur_pos) {
        leftChar();
    }
}


void CmdHistory::searchUp()
{
    // Don't do anything when at top.
    if (hist_pos == 0)
        return;
    size_t new_pos = hist_pos;
    bool match = false;
    // Walk backwards in history until hit bottom or match.
    do {
        match = true;
        std::string line = history[--new_pos];
        for (size_t i = 0; i < buf_ptr; i++) {
            if (buffer[i] != line[i]) {
                match = false;
                break;
            }
        }
    } while (!match && new_pos != 0);

    // If we did not find it, don't change anything.
    if (!match) {
        return;
    }
    hist_pos = new_pos;
    // Need to move one forward since match would have pointed to
    // previous entry.

    // Clear the line
    send_esc("\r\033[0K");
    // Save current position.
    size_t cur_pos = buf_ptr;
    pos = 0;
    // Print out prompt.
    send(prompt);
    // Grab last history line.
    std::string line = history.at(hist_pos);
    size_t len = line.size();
    buf_ptr = 0;
    buf_end = sizeof(buffer)-1;
    // Copy over to buffer.
    for(size_t i=0; i < len; i++) {
        buffer[buf_ptr] = line[i];
        send_char->notify((void *)&buffer[buf_ptr]);
        buf_ptr++;
        pos++;
    }
    // Put cursor in about same place.
    while(buf_ptr > cur_pos) {
        leftChar();
    }
}

void CmdHistory::moveDown()
{
    if (multiline) {
        // Handle multiline history.
    }
    // Don't do anything when at top.
    if (hist_pos == history.size())
        return;
    // Move to next history entry.
    hist_pos ++;
    // Clear the line
    send_esc("\r\033[0K");
    // Save current position.
    size_t cur_pos = buf_ptr;
    pos = 0;
    // Print out prompt.
    send(prompt);
    // If at end of history, create blank line.
    if (hist_pos == history.size()) {
        clear_line();
        return;
    }
    // Grab last history line.
    std::string line = history.at(hist_pos);
    size_t len = line.size();
    buf_ptr = 0;
    buf_end = sizeof(buffer)-1;
    // Copy over to buffer.
    for(size_t i=0; i < len; i++) {
        buffer[buf_ptr] = line[i];
        send_char->notify((void *)&buffer[buf_ptr]);
        buf_ptr++;
        pos++;
    }
    // Put cursor in about same place.
    while(buf_ptr > cur_pos) {
        leftChar();
    }
}

void CmdHistory::searchDown()
{
    // Don't do anything when at top.
    if (hist_pos == history.size())
        return;
    size_t new_pos = hist_pos + 1;
    bool match = false;
    // Walk backwards in history until hit bottom or match.
    while (new_pos < history.size() && !match) {
        match = true;
        std::string line = history[new_pos];
        new_pos++;
        for (size_t i = 0; i < buf_ptr; i++) {
            if (buffer[i] != line[i]) {
                match = false;
                break;
            }
        }
    }

    // If we did not find it, don't change anything.
    if (!match) {
        return;
    }

    // Move found history item
    hist_pos = new_pos - 1;
    // Clear the line
    send_esc("\r\033[0K");
    // Save current position.
    size_t cur_pos = buf_ptr;
    pos = 0;
    // Print out prompt.
    send(prompt);
    // If at end of history, create blank line.
    if (hist_pos == history.size()) {
        clear_line();
        return;
    }
    // Grab last history line.
    std::string line = history.at(hist_pos);
    size_t len = line.size();
    buf_ptr = 0;
    buf_end = sizeof(buffer)-1;
    // Copy over to buffer.
    for(size_t i=0; i < len; i++) {
        buffer[buf_ptr] = line[i];
        send_char->notify((void *)&buffer[buf_ptr]);
        buf_ptr++;
        pos++;
    }
    // Put cursor in about same place.
    while(buf_ptr > cur_pos) {
        leftChar();
    }
}

void CmdHistory::recv_key(const CmdKey *key)
{
    //std::cerr << "Command: " << key->ch << std::endl;
    switch(key->key) {
    case KeyType::ascii:
        insertChar(key->ch);
        break;
    case KeyType::up:
        moveUp();
        break;
    case KeyType::down:
        moveDown();
        break;
    case KeyType::left:
        leftChar();
        break;
    case KeyType::right:
        rightChar();
        break;
    case KeyType::backspace:
        backSpace();
        break;
    case KeyType::cancel:
        cancelLine();
        break;
    case KeyType::bol:
        moveBol();
        break;
    case KeyType::eol:
        moveEol();
        break;
    case KeyType::ignore:
        break;
    case KeyType::del:
        deleteNext();
        break;
    case KeyType::complete:
        break;
    case KeyType::accept:
        acceptLine();
        wru(true);
        break;
    case KeyType::kill:
        deleteEol();
        break;
    case KeyType::search_up:
        searchUp();
        break;
    case KeyType::search_down:
        searchDown();
        break;
    case KeyType::position:
        break;
    case KeyType::eof:
        break;
    };

}

void CmdHistory::wru(const bool mode)
{
    if (mode) {
        send("\n\r");
        pos = 0;
        send(prompt);
        hist_pos = history.size();
        clear_line();
    } else {
        std::cerr << "Command WRU: " << mode << std::endl;
    }
}


void CmdHistory::clear_line()
{
    buf_ptr = 0;
    buf_end = sizeof(buffer) - 1;
    buffer[0] = '\0';
    multiline = false;
    row = 0;
}

void CmdHistory::refresh()
{
}


}
