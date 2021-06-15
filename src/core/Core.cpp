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

#include <ctype.h>
#include <string>
#include <vector>
#include "Core.h"

bool compareChar(const char & c1, const char & c2)
{
    if (c1 == c2)
        return true;
    else if (std::toupper(c1) == std::toupper(c2))
        return true;
    return false;
}

bool stringCompare(const std::string & str1, const std::string & str2)
{
    return ((str1.size() == str2.size()) &&
        std::equal(str1.begin(), str1.end(), str2.begin(), &compareChar));
}

bool findString(const std::vector<std::string> list, const std::string & str )
{
    if (list.size() == 0) {
        return true;
    }
    for(auto & s : list) {
        if(stringCompare(s, str)) {
            return true;
        }
    }
    return false;
}

int hextoint(const char & c)
{
    switch(c) {
    case '0':
            return 0;
    case '1':
            return 1;
    case '2':
            return 2;
    case '3':
            return 3;
    case '4':
            return 4;
    case '5':
            return 5;
    case '6':
            return 6;
    case '7':
            return 7;
    case '8':
            return 8;
    case '9':
            return 9;
    case 'a':
    case 'A':
            return 0xa;
    case 'b':
    case 'B':
            return 0xb;
    case 'c':
    case 'C':
            return 0xc;
    case 'd':
    case 'D':
            return 0xd;
    case 'e':
    case 'E':
            return 0xe;
    case 'f':
    case 'F':
            return 0xf;
    default:
            break;
    }
    return 0x10;
}
