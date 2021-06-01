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
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "Options.h"
#include "CppUTest/TestHarness.h"

using namespace std;

TEST_GROUP(OptionParser)
{
};

    TEST(OptionParser, Create) {
        option::OptionParser op("example");
        CHECK(op.getDescription() == "example");
    }
    
    TEST(OptionParser, Create2) {
        option::OptionParser op("Allowed options");
        auto help_option = op.add<option::OptionSwitch>("h", "help", "produce help message");
        cout << op.help() << endl;
    }
    
    TEST(OptionParser, Option1) {
        option::OptionParser op("Allowed2 options");
        auto help_option = op.add<option::OptionSwitch>("h", "help", "produce help message");
        auto int_option = op.add<option::OptionValue<int>>("i", "int", "test integer");
        const char *args[3] = { "prog", "-h", NULL };
        op.parse(2, args);
        CHECK_EQUAL(help_option->is_set(), true);
    }
    
    TEST(OptionParser, Option2) {
        option::OptionParser op("Allowed3 options");
        auto help_option = op.add<option::OptionSwitch>("h", "help", "produce help message");
        auto int_option = op.add<option::OptionValue<int>>("i", "int", "test integer");
        const char *args[4] = { "prog", "-i", "42", NULL };
        op.parse(3, args);
        CHECK_EQUAL(int_option->is_set(), true);
        CHECK_EQUAL(help_option->is_set(), false);
        CHECK_EQUAL(int_option->getValue(), 42);
    }
