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

#include "i8080_system.h"
#include "Memory.h"
#include "RAM.h"
#include "ROM.h"

using namespace std;
using namespace core;

map<string, SystemFactory *> System::factories;
REGISTER_SYSTEM(i8080)

map<string, MemFactory *>    i8080::mem_factories;
map<string, IOFactory *>     i8080::io_factories;
map<string, DeviceFactory *> i8080::dev_factories;

REGISTER_MEM(i8080, RAM, uint8_t);
REGISTER_MEM(i8080, ROM, uint8_t);
