#include <ctype.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Config.h"
#include "ConfigLexer.h"

namespace core
{
using namespace std;

/**
 * @class Keyword
 * @author rich
 * @date 25/05/21
 * @file ConfigLexer.cpp
 * @brief List of keyword string and returned value.
 */
struct Keyword {
    string      name;
    ConfigToken value;
};

vector<Keyword> Keywords = {
    {"system", ConfigToken::Sys},
    {"cpu", ConfigToken::Cpu},
    {"memory", ConfigToken::Mem},
    {"device", ConfigToken::Dev},
    {"unit", ConfigToken::Unit},
    {"control", ConfigToken::Ctl},
    {"units", ConfigToken::Units},
    {"load", ConfigToken::Load},
    {"mount", ConfigToken::Mount},
    {"ro", ConfigToken::RO},
};


/**
 * @brief Default Constructor.
 * @param is
 */
ConfigLexer::ConfigLexer(std::istream& is)
    : p_input{&is}, owns_input{false}, cur_token_value{0}, value{0}
{
    init();
}

/**
 * @brief Default Constructor.
 * @param ps
 */
ConfigLexer::ConfigLexer(std::istream *ps)
    : p_input{ps}, owns_input{true}, cur_token_value{0}, value{0}
{
    init();
}

/**
 * @brief Initialize the object.
 */
void ConfigLexer::init()
{
}

/**
 * @brief Advance to the next token. Setting cur_token_text to
 *     value of last match string and cur_token_value to the last
 *     matched number.
 * @param keyword Indicates whether to scan for keywords or not.
 */
void ConfigLexer::advance(bool keyword)
{
    if (cur_token != ConfigToken::EOFSym) {
        cur_token = get_token(keyword);
        cur_token_text = buffer;
        cur_token_value = value;
    }
}

/**
 * @brief Advance to the next token.
 * @detail Tokens are a series of letters followed by numbers and _.
 *         If not looking for a keyword, numbers are considered identifiers.
 *         Numbers can be of the format 0xnn, nnh, 0nn nno nnb
 *         Numbers can be followed by an optional 'k', 'm' or 'g' indicating
 *            scale factor.
 *         All other symbols are returned as a keyword, or throw an exception.
 * @param keyword Whether to scan for keywords or not.
 * @return next token.
 */
ConfigToken ConfigLexer::get_token(bool keyword)
{
    std::istream& input = *p_input;
    buffer.clear();
    value = 0;
    char c = input.get();

    // Skip end leading white space.
    while (isspace(c)) c = input.get();
    if (!input) return ConfigToken::EOFSym;

    // See if this is a identifier or keyword.
    if (isalpha(c) || (!keyword && isdigit(c))) {
        buffer = tolower(c);
        c = input.get();

        // Grab all alphanumeric or _.
        while (isalnum(c) || c == '_') {
            buffer += tolower(c);
            c = input.get();
        }

        input.putback(c);

        // If not looking for keywords, return an Id.
        if (!keyword)
            return ConfigToken::Id;

        // Check if possible keyword.
        for(const auto &id : Keywords) {
            if (buffer == id.name)
                return id.value;
        }
        return ConfigToken::Id;
    }

    // Check if it could be a string.
    if (c == '"') {
        c = input.get();
        while (c != '"') {
            buffer += c;
            c = input.get();
            // Check if double ", which becomes single.
            if (c == '"') {
                if (input.peek() == '"') {
                    buffer += c;
                    c = input.get();
                } else {
                    break;
                }
            }
        }
        return ConfigToken::Str;
    }

    // Check if special character.
    if (c == '#') return ConfigToken::EOFSym;
    if (c == '(') return ConfigToken::Rparn;
    if (c == ')') return ConfigToken::Lparn;
    if (c == ':') return ConfigToken::Colon;
    if (c == '=') return ConfigToken::Equal;
    if (c == ',') return ConfigToken::Comma;

    // Perhaps a number?
    if (isdigit(c)) {
        size_t base = 10;
        size_t scale = 1;
        // Check for leading 0x.
        if (c == '0') {
            c = tolower(input.get());
            if (c == 'x') { // Hex number.
                base = 16;
                c = input.get();
            } else { // Octal number?
                base = 8;
            }
        }
        // Slurp up digits until none digit.
        buffer += c;
        c = input.get();
        bool last_b = false;
        while (isxdigit(c)) {
            buffer += c;
            c = input.get();
            // Look for binary tag.
            last_b = (tolower(c) == 'b');
        }
        if (last_b && base != 16)
            base = 2;
        // Check next character, possible base..
        c = tolower(c);
        if (c == 'h') {
            base = 16;
            c = input.get();
        } else if (c == 'o') {
            base = 8;
            c = input.get();
        }

        // Check if possible scale.
        if (c == 'k') {
            scale = 1024;
            c = input.get();
        } else if (c == 'm') {
            scale = 1024 * 1024;
            c = input.get();
        } else if (c == 'g') {
            scale = 1024 * 1024 * 1024;
            c = input.get();
        }

        // Not something we want save it for next call.
        input.putback(c);

        // Convert the number based on base given to a value.
        value = 0;
        for (auto &ch : buffer) {
            const string hexChars = "0123456789abcdef";
            const size_t pos = hexChars.find(ch);
            if (pos == string::npos)
                throw Lexical_error{"Invalid digit: " + ch};
            if (pos >= base)
                throw Lexical_error{"Digit out of range: " + ch};
            value = (value * base) + pos;
        }

        // Scale by given scale.
        value = value * scale;
        return ConfigToken::Number;
    }
    return ConfigToken::EOFSym;
}
}
