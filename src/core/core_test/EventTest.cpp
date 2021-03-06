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

#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include "Event.h"
#include "CppUTest/TestHarness.h"

using namespace core;
using namespace std;

class Event_test
{
    public:
    void uselessFunction(void *v) {
        int val = *((int *)v);
        cerr << "Useless called " << val << endl;
    }
};

TEST_GROUP(EventTest)
{
};


TEST(EventTest, Create) {
        Event_test* test = new Event_test();
        EventCallback<Event_test>* callback = new
            EventCallback<Event_test>(test, &Event_test::uselessFunction);
        Event* ev = new Event();
        ev->addListener(callback);
        int v = 16;
        ev->notify((void *)&v);

        delete callback;
        delete test;
        delete ev;
}

