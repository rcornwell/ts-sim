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
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "ConfigLexer.h"
#include "CppUTest/TestHarness.h"

TEST_GROUP(ConfigLexer)
{
};


TEST(ConfigLexer, Create) {
	std::istringstream ist{""};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

TEST(ConfigLexer, Blanks) {
	std::istringstream ist{"   "};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

    TEST(ConfigLexer, System) {
        std::istringstream ist{" system"};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::Sys);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

    TEST(ConfigLexer, System2) {
	    std::istringstream ist{"system test   "};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::Sys);
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::Id);
        CHECK_EQUAL(lexer->token_text(), "test");
        delete lexer ;
    }

    TEST(ConfigLexer, Number) {
	    std::vector<uint64_t> v { 12, 077, 0x40, 0xaf, 0xbe, 5, 0x9a };
	    std::istringstream ist{"12 077 0x40 0afh 0xBE 0101B 9aH "};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        for(uint64_t i : v) {
            CHECK(lexer->token() == core::ConfigToken::Number);
            CHECK_EQUAL(lexer->token_value(), i);
            lexer->advance();
        }
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

    TEST(ConfigLexer, Tokens) {
        using namespace core;
	std::vector<ConfigToken> v {
            ConfigToken::Id, ConfigToken::Str, ConfigToken::Sys,
            ConfigToken::Cpu, ConfigToken::Dev, ConfigToken::Unit,
            ConfigToken::Ctl, ConfigToken::Units, ConfigToken::Number,
            ConfigToken::Rparn, ConfigToken::Lparn, ConfigToken::Colon,
            ConfigToken::Equal, ConfigToken::Comma, ConfigToken::Load,
            ConfigToken::Mount, ConfigToken::RO, ConfigToken::EOFSym
        };
	std::istringstream ist{"test \"test2\" system cpu device unit control units 032 ():=, load mount ro"};
        ConfigLexer *lexer = new ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        for(ConfigToken i : v) {
            CHECK(lexer->token() == i);
            lexer->advance();
        }
        CHECK(lexer->token() == ConfigToken::EOFSym);
        delete lexer;
    }
    
    TEST(ConfigLexer, Tokens2) {
        using namespace core;
	std::vector<ConfigToken> v {
            ConfigToken::Id, ConfigToken::Str, ConfigToken::Sys,
            ConfigToken::Cpu, ConfigToken::Dev, ConfigToken::Unit,
            ConfigToken::Ctl, ConfigToken::Units, ConfigToken::Number,
            ConfigToken::Rparn, ConfigToken::Lparn, ConfigToken::Colon,
            ConfigToken::Equal, ConfigToken::Comma, ConfigToken::Load,
            ConfigToken::Mount, ConfigToken::RO, ConfigToken::EOFSym
        };
	std::istringstream ist{"TEST \"TEST2\" SYSTEM CPU DEVICE UNIT CONTROL UNITS 032 ():=, LOAD MOUNT RO"};
        ConfigLexer *lexer = new ConfigLexer{ist};
        CHECK(lexer != nullptr);
        lexer->advance();
        for(ConfigToken i : v) {
            CHECK(lexer->token() == i);
            lexer->advance();
        }
        CHECK(lexer->token() == ConfigToken::EOFSym);
        delete lexer;
    }
    
    TEST(ConfigLexer, Errors) {
	std::istringstream ist{"1a2 079 "};
        core::ConfigLexer *lexer = new core::ConfigLexer{ist};
        CHECK(lexer != nullptr);
        CHECK_THROWS(core::Lexical_error, lexer->advance());
        CHECK_THROWS(core::Lexical_error, lexer->advance());
        lexer->advance();
        CHECK(lexer->token() == core::ConfigToken::EOFSym);
        delete lexer;
    }

