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
#include <stdio.h>
#include <string>
#include "Command.h"
#include "Console.h"

namespace core
{

class CmdHistory
{
public:
    CmdHistory()
    {
        buf_ptr = 0;
        buf_end = sizeof(buffer) - 1;
    }

    virtual ~CmdHistory()
    {
    }

    virtual void init();

    void send(const std::string & str);

    void send_esc(const std::string & str);

    void leftChar();

    void rightChar();
    
    void moveBol();

    void moveEol();

    void backSpace();

    void cancelLine();

    void deleteNext();

    void deleteEol();

    void insertChar(char ch);

    void acceptLine();
    
    void moveUp();

    void searchUp();

    void moveDown();

    void searchDown();

    void recv_key(const CmdKey *key);

    void wru(const bool mode);
    
    void clear_line();

    void refresh();

    Command            *cmd;
    Console            *con;
    Event              *send_char;
    int                 pos;
    char                buffer[1024];
    size_t              buf_ptr;            // Index into buffer.
    size_t              buf_end;            // Index end of buffer.
    std::string         prompt{"sim> "};    // Prompt string.
    std::vector<std::string>   history;     // Holds history record.
    size_t              hist_pos;
    bool                multiline;
    int                 row;                // Command row.
};

}
