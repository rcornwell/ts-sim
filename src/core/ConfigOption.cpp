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


#include "ConfigOption.h"

namespace core
{

/**
 * @brief Specific parser for strings.
 * @detail String options are name="value" followed by optional comma.
 * @param p_lexer
 */
template<>
void ConfigValue<std::string>::parse(ConfigLexer *p_lexer)
{
    if (p_lexer->token() != ConfigToken::Equal)
        throw Config_error{"Option not followed by equal: " + name_};
    p_lexer->advance();
    if (p_lexer->token() != ConfigToken::Str)
        throw Config_error{"Not a String: " + name_ };
    add_value(p_lexer->token_text());
    p_lexer->advance();
}

/**
 * @brief Specific parser for numbers.
 * @detail Numeric options are name=# followed by optional comma.
 * @param p_lexer
 */
template<>
void ConfigValue<int>::parse(ConfigLexer *p_lexer)
{
    if (p_lexer->token() != ConfigToken::Equal)
        throw Config_error{"Option not followed by equal: " + name_};
    p_lexer->advance();
    if (p_lexer->token() != ConfigToken::Number)
        throw Config_error{"Not a number: " + name_ };
    add_value((int)p_lexer->token_value());
    p_lexer->advance();
}

/**
 * @brief Specific parser for numbers (size_t).
 * @detail Numeric options are name=# followed by optional comma.
 * @param p_lexer
 */
template<>
void ConfigValue<size_t>::parse(ConfigLexer *p_lexer)
{
    if (p_lexer->token() != ConfigToken::Equal)
        throw Config_error{"Option not followed by equal: " + name_};
    p_lexer->advance();
    if (p_lexer->token() != ConfigToken::Number)
        throw Config_error{"Not a number: " + name_ };
    add_value(p_lexer->token_value());
    p_lexer->advance();
}

/**
 * @brief Specific parser for boolean.
 * @detail Boolean options are just name followed by optional comma.
 * @param p_lexer
 */
template<>
void ConfigValue<bool>::parse([[maybe_unused]]ConfigLexer *p_lexer)
{
    add_value(true);
    update_reference();
}

/**
 * @brief Specific parser for boolean.
 * @detail Boolean options are just name followed by optional comma.
 * @param p_lexer
 */
void ConfigBool::parse([[maybe_unused]]ConfigLexer *p_lexer)
{
    setValue(true);
    update_reference();
}

/**
 * @brief Parser for multioption maps.
 * @detail Maps are name=value. 
 * @param p_lexer
 */
template<typename T>
void ConfigMap<T>::parse(ConfigLexer *p_lexer)
{
    if (p_lexer->token() != ConfigToken::Equal)
        throw Config_error{"Option not followed by equal: " + this->name_};
    p_lexer->advance(false);
    if (p_lexer->token() != ConfigToken::Id)
        throw Config_error{"Not a term: " + this->name_ };
    add_value(p_lexer->token_text());
    p_lexer->advance();
}

/**
 * @brief Default constructor.
 * @param description
 */
ConfigOptionParser::ConfigOptionParser(std::string description) :
    description_(std::move(description))
{
}

   /**
     * @brief Get all options added.
     * @return Vector of options.
     */
const std::vector<ConfigOption_ptr>& ConfigOptionParser::getOptions() const
{
    return options_;
}

   /**
     * @brief Find a option based on name.
     * @param name - name of option
     * @return Pointer to option.
     */
ConfigOption_ptr ConfigOptionParser::find_option(const std::string& name) const
{
    for (const auto& option: options_)
        if (option->getName() == name)
            return option;
    return nullptr;
}

/**
 * @brief Parse a series of options until a ")" is found.
 * @detail options are name or name=value followed by optional comma.
 * @param p_lexer
 */
void ConfigOptionParser::parse(ConfigLexer* p_lexer)
{
    for (auto& opt : options_)
        opt->clear();
    p_lexer->advance(false);
    // Look for string of ID's
    while (p_lexer->token() == ConfigToken::Id) {
        // See if we can find it.
        ConfigOption_ptr option = find_option(p_lexer->token_text());
        // Call suboption processor to set it.
        if (option) {
            p_lexer->advance();
            option->parse(p_lexer);
        } else {
            throw Config_error{"Invalid Option: " + p_lexer->token_text()};
        }
        if (p_lexer->token() == ConfigToken::Comma)
            p_lexer->advance();
    }
    if (p_lexer->token() != ConfigToken::Lparn)
        throw Config_error{"Options not ended with a )"};
    p_lexer->advance(); // Eat ).
    // Check that required options have been set.
}

}
