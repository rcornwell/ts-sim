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

/* This code is based on POPL option parsing by Johannes Pohl.
 * This code was orignally released under MIT license. This code
 * has been changed to reduce number of type of options.
 */

#pragma once

#include <string>
#include <iostream>
#include <ostream>
#include <sstream>
#include <memory>
#include <vector>
#include <cstring>

namespace option
{

/**
* @brief defines type of argument for each option.
*/
enum class Argument {
    no = 0,		// option never takes an argument
    required,	// option always take oargemnet
    optional	// option can take an argument
};

/**
 * @brief defines whether option is not shown, required or optional.
 */
enum class Attribute {
    hidden = 0,
    required,
    optional
};

/**
 * @brief defines type of option to report for errors.
 */
enum class OptionType {
    noOption,
    shortOption,
    longOption
};

/**
 * @class Option
 * @author rich
 * @date 27/05/21
 * @file Options.h
 * @brief Holder class for options. This is an abstract class.
 */
class Option
{
    friend class OptionParser;
public:

    /**
     * @brief Default constructor.
     * @param short_name
     * @param long_name
     * @param description
     */
    Option(const std::string& short_name, const std::string &long_name,
           const std::string& description);

    /**
     * @brief Use default destructor.
     */
    virtual ~Option() = default;

    /**
     * @brief Default copy constructor.
     */
    Option(const Option&) = default;

    /**
     * @brief Default move constructor.
     */
    Option(Option&&) = default;

    /**
     * @brief Default assignment operator.
     */
    Option& operator=(const Option&) = default;

    /**
     * @brief Default move assignment operator.
     */
    Option& operator=(Option&&) = default;

    /**
     * @brief Returns the short name assigned to this option.
     * @return short_name
     */
    char getShortName() const
    {
        if (!short_name_.empty())
            return short_name_[0];
        return 0;
    }

    /**
     * @brief Returns the long name assigned to this option.
     * @return long_name
     */
    std::string getLongName() const
    {
        return long_name_;
    }

    /**
     * @brief Used by error routine to return the desired option name.
     * @param type - short or long name.
     * @param with_hypen - whether to preappend the hyphen character or not.
     * @return option with optional hyphen in front.
     */
    std::string getName(OptionType type, bool with_hypen) const
    {
        if (type == OptionType::shortOption)
            return short_name_.empty() ? "" : ((with_hypen? "-" : "") + short_name_);
        if (type == OptionType::longOption)
            return long_name_.empty() ? "" : ((with_hypen? "--" : "") + long_name_);
        return "";
    }

    /**
     * @brief Return the description of this option.
     * @return Description.
     */
    std::string getDescription() const
    {
        return description_;
    }

    /**
     * @brief Set the attribute associated with this option.
     * @param attribute
     */
    void setAttribute(const Attribute& attribute)
    {
        attribute_ = attribute;
    }
    
    /**
     * @brief Return the attribute associated with this option.
     * @detail Attribute determines if option is needed or not.
     * @return Attribute
     */
    Attribute getAttribute() const
    {
        return attribute_;
    }

    /**
     * @brief Get the default value associated with this option.
     * @param out string containting default value.
     * @return bool indicating whether default was set or not.
     */
    virtual bool getDefault(std::ostream& out) const = 0;

    /**
     * @brief Return whether this option takes an argument or not.
     * @return Argument.
     */
    virtual Argument getArgument() const = 0;

    /**
     * @brief return number of times this option was seen.
     */
    virtual size_t count() const = 0;

    /**
     * @brief return whether option has been set or not.
     */
    virtual bool is_set() const = 0;

protected:
    /**
     * @brief Parse this option.
     * @param what_type whether long or short type.
     * @param value Value to set option too.
     */
    virtual void parse(OptionType what_type, const char *value) = 0;

    /**
     * @brief rest option to never been set.
     */
    virtual void clear() = 0;

    // Holds short name of option.
    std::string short_name_;
    // Holds long name of option.
    std::string long_name_;
    // Holds description of option.
    std::string description_;
    // Holds the type of option this is.
    Attribute attribute_;
};


/**
 * @class invalid_option
 * @author rich
 * @date 27/05/21
 * @file Options.h
 * @brief Handle invalid option settings.
 */
class invalid_option : public std::invalid_argument
{
public:
    enum class Error {
        missing_argument,
        invalid_argument,
        too_many_arguments,
        missing_option
    };

    /**
     * @brief Standard constructor.
     * @param option - Option in error.
     * @param error - type of error.
     * @param what_type - whether short or long specified.
     * @param value - value trying to use.
     * @param text - message to append to error.
     */
    invalid_option(const Option* option, invalid_option::Error error,
                   OptionType what_type, std::string value, const std::string& text) :
        std::invalid_argument(text.c_str()), option_(option), error_(error),
        what_type_(what_type), value_(std::move(value))
    {
    }

    /**
     * @brief Short constructor for when value not being set.
     * @param option - option in question.
     * @param error - error type.
     * @param text - message to append to error.
     */
    invalid_option(const Option* option, invalid_option::Error error,
                   const std::string& text) :
        invalid_option(option, error, OptionType::noOption, "", text)
    {
    }

    /**
     * @brief Return option in question.
     * @return option
     */
    const Option* getOption() const
    {
        return option_;
    }

    /**
     * @brief Return error code.
     * @return error code.
     */
    Error getError() const
    {
        return error_;
    }

    /**
     * @brief Return type of option.
     * @return option type.
     */
    OptionType getWhatType() const
    {
        return what_type_;
    }

    /**
     * @brief Return value assigned.
     * @return
     */
    std::string getValue() const
    {
        return value_;
    }

private:
    const Option* option_;
    Error error_;
    OptionType what_type_;
    std::string value_;

};

/**
 * @class OptionValue
 * @author rich
 * @date 27/05/21
 * @file Options.h
 * @brief Generic option value.
 */
template<typename T>
class OptionValue : public Option
{
public:
    /**
     * @brief Default constructor.
     * @param short_name - short name for this option.
     * @param long_name - long name for this option.
     * @param description - description of this option.
     */
    OptionValue(const std::string& short_name, const std::string& long_name,
                const std::string& description)
        : Option(short_name, long_name, description),
          assign_to_(nullptr)
    {
    }

    /**
     * @brief Full constructor.
     * @param short_name - short name for this option.
     * @param long_name - long name for this option.
     * @param description - description of this option.
     * @param default_val - default value to set if option not given.
     * @param assign_to - location to assign results too.
     */
    OptionValue(const std::string& short_name, const std::string& long_name,
                const std::string& description,
                const T& default_val, T* assign_to = nullptr)
        : OptionValue<T>(short_name, long_name, description)
    {
        assign_to_ = assign_to;
        setDefault(default_val);
    }

    /**
     * @brief return number of times this option was seen.
     */
    size_t count() const override
    {
        return count_;
    }

    /**
     * @brief return whether option has been set or not.
     */
    bool is_set() const override
    {
        return count_ != 0;
    }

    /**
     * @brief Set location to assign value to.
     * @param var location to set with result.
     */
    void assign_to(T* var)
    {
        assign_to_ = var;
        update_reference();
    }


    /**
     * @brief Set the value of this option to value.
    * @param value - Value to set.
     */
    void setValue(const T* value)
    {
        clear();
        add_value();
    }


    /**
     * @brief Get the value this option is currently set to.
     */
    T getValue() const
    {
        // If not set and has default value, return default.
        if(!this->is_set() && default_)
            return *default_;
        // If not set and no default, throw exception.
        if (!this->is_set()) {
            std::stringstream optionStr;
            optionStr << "option not set: \"";
            // construct message to send.
            if (getShortName() != 0)
                optionStr << "-" << getShortName();
            else
                optionStr << "--" << getLongName();
            optionStr << "\"";
            throw std::out_of_range(optionStr.str());
        }
        return value_;
    }

    /**
     * @brief Set the default value. This will update assigned to if set.
     * @param value - value to set.
     */
    void setDefault(const T& value)
    {
        // Clear variable and reset it.
        this->default_.reset(new T);
        *this->default_ = value;
        // Update assigned value.
        update_reference();
    }


    /**
     * @brief Return whether there is currently a default value for this option.
     * @return bool - true default, false no default.
     */
    bool hasDefault() const
    {
        return (this->default_ != nullptr);
    }

    /**
     * @brief Get the default value associated with this option.
     * @param out string containting default value.
     * @return bool indicating whether default was set or not.
     */
    T getDefault() const
    {
        if (!hasDefault())
            throw std::runtime_error("No default value set");
        return *this->default_;
    }
    
    virtual bool getDefault(std::ostream& out) const override
    {
        if(!hasDefault())
            return false;
        out << *this->default_;
        return true;
    }

    /**
     * @brief Return whether this option takes an argument or not.
     * @return Argument.
     */
    Argument getArgument() const override
    {
        return Argument::required;
    }

protected:
    /**
     * @brief Parse this option.
     * @param what_type
     * @param value
     */
    void parse(OptionType what_type, const char* value) override
    {
        T parsed_value;
        std::string strValue;
        // Set strValue to argument if there is one.
        if (value != nullptr)
            strValue = value;

        std::istringstream is(strValue);
        int valuesRead = 0;
        // If stream ok, see if we can get a value of type T.
        while (is.good()) {
            if (is.peek() != EOF)
                is >> parsed_value;
            else
                break;
            valuesRead++;
        }

        // We could not throw exception.
        if (is.fail())
            throw invalid_option(this, invalid_option::Error::invalid_argument, what_type,
                                 value, "invalid argument for " + getName(what_type, true) + ": '" + strValue + "'");

        // We only support one argument per option.
        if (valuesRead > 1)
            throw invalid_option(this, invalid_option::Error::too_many_arguments, what_type,
                                 value, "too many arguments for " + getName(what_type, true) + ": '" + strValue + "'");

        // If the string is empty, then no argument.
        if (strValue.empty())
            throw invalid_option(this, invalid_option::Error::missing_argument, what_type, "",
                                 "missing argument for " + getName(what_type, true));

        // All ok, set it.
        this->add_value(parsed_value);
    }

    /**
     * @brief Called when value changes to update assigned value.
     */
    virtual void update_reference()
    {
        if (this->assign_to_) {
            if (this->is_set() || default_)
                *this->assign_to_ = getValue();
        }
    }

    /**
     * @brief Update the value and references.
     * @param value
     */
    virtual void add_value(const T & value)
    {
        value_ = value;
        count_++;
        update_reference();
    }
    
    /**
     * @brief Mark option as unset.
     */
    virtual void clear() override
    {
        count_ = 0;
        update_reference();
    }

    // Holds default value.
    std::unique_ptr<T> default_;
    // Holds place to save setting to.
    T* assign_to_;
    // Holds last value seen.
    T value_;
    // Holds number of times this option has been seen.
    size_t count_;
};


template <typename T>
/**
 * @class OptionImplicit
 * @author rich
 * @date 27/05/21
 * @file Options.h
 * @brief These options always are set, either explicity or via default value.
 */
class OptionImplicit : public OptionValue<T>
{
public:
    /**
     * @brief Full constructor.
     * @param short_name - short name for this option.
     * @param long_name - long name for this option.
     * @param description - description of this option.
     * @param default_val - default value to set if option not given.
     * @param assign_to - location to assign results too.
     */
    OptionImplicit(const std::string& short_name, const std::string& long_name,
                   const std::string& description,
                   const T& default_val, T* assign_to = nullptr)
        :
        OptionValue<T>(short_name, long_name, description, default_val, assign_to)
    {
    }

    /**
     * @brief Return whether this option takes an argument or not.
     * @return Argument.
     */
    Argument getArgument() const override
    {
        return Argument::optional;
    }

protected:
    /**
      * @brief Parse this option.
      * @param what_type
      * @param value
      */
    void parse(OptionType what_type, const char* value) override
    {
        if ((value != nullptr) && (strlen(value) > 0))
            OptionValue<T>::parse(what_type, value);
        else
            this->add_value(*this->default_);

    }
};


/**
 * @class OptionSwitch
 * @author rich
 * @date 27/05/21
 * @file Options.h
 * @brief Switch options do not take any arguments.
 */
class OptionSwitch : public OptionValue<bool>
{
public:
    OptionSwitch(const std::string& short_name, const std::string& long_name,
                 const std::string& description,
                 bool* assign_to = nullptr)
        :
        OptionValue<bool>(short_name, long_name, description, false, assign_to)
    {
    }

    // No defualt arguments.
    void setDefault(const bool& value) = delete;

    /**
      * @brief Return whether this option takes an argument or not.
      * @return Argument.
      */
    Argument getArgument() const override
    {
        return Argument::no;
    }

protected:
    /**
      * @brief Parse this option.
      * @param what_type
      * @param value
      */
    void parse([[maybe_unused]]OptionType what_type, [[maybe_unused]]const char* value) override
    {
        this->add_value(true);
    }

};

using Option_ptr = std::shared_ptr<Option>;

/**
 * @class OptionParser
 * @author rich
 * @date 27/05/21
 * @file Options.h
 * @brief Main class to hold options and parse command line.
 */
class OptionParser
{
public:
    /**
     * @brief Default constructor.
     * @param description Description of program.
     */
    explicit OptionParser(std::string description = "")
        : description_(std::move(description))
    {
    }

    // Use default destructor.
    virtual ~OptionParser() = default;

    /**
     * @brief Add option to be scanned.
     * @return
     */
    template<typename T, Attribute attribute, typename... Ts>
    std::shared_ptr<T> add(Ts&&... params)
    {
        static_assert(
            std::is_base_of<Option, typename std::decay<T>::type>::value,
            "type T must be Switch, Value or Implicit"
        );
        std::shared_ptr<T> option = std::make_shared<T>(std::forward<Ts>(params)...);

        for (const auto& o: options_) {
            if ((option->getShortName() != 0) && (option->getShortName() == o->getShortName()))
                throw std::invalid_argument("duplicate short option name '-" + std::string(1, option->getShortName()) + "'");
            if (!option->getLongName().empty() && (option->getLongName() == (o->getLongName())))
                throw std::invalid_argument("duplicate long option name '--" + option->getLongName() + "'");
        }
        option->setAttribute(attribute);
        options_.push_back(option);
        return option;
    }

    /**
     * @brief Add optional option.
     * @return
     */
    template<typename T, typename... Ts>
    std::shared_ptr<T> add(Ts&&... params)
    {
        return add<T, Attribute::optional>(std::forward<Ts>(params)...);
    }

    /**
     * @brief Parse the command line arguments.
     * @param argc - number of arguments.
     * @param argv - array of values.
     */
    void parse(int argc, const char * const argv[]);

    /**
     * @brief Print help message.
     */
    std::string help() const;

    /**
     * @brief Return the description.
     * @return
     */
    std::string getDescription() const
    {
        return description_;
    }

    /**
     * @brief Return list of arguments after the --
     */
    const std::vector<std::string>& non_option_args() const
    {
        return non_option_args_;
    }

    /**
     * @brief Return list of all arguments that could not be found.
     */
    const std::vector<std::string>& unknown_options() const
    {
        return unknown_options_;
    }
    
    /**
    * @brief Get all options added.
    * @return Vector of options.
    */
    const std::vector<Option_ptr>& getOptions() const
    {
        return options_;
    }

    /**
     * @brief return option by long name.
     * @param long_name
     * @return option object.
     */
    template<typename T>
    std::shared_ptr<T> getOption(const std::string& long_name) const
    {
        Option_ptr option = find_option(long_name);
        if (!option)
            throw std::invalid_argument("option not found: " + long_name);
        auto result = std::dynamic_pointer_cast<T>(option);
        if (!result)
            throw std::invalid_argument("cannot cast option to T: " + long_name);
        return result;
    }

    /**
     * @brief Return an option by short name.
     * @param short_name
     * @return
     */
    template<typename T>
    std::shared_ptr<T> getOption(char short_name) const
    {
        Option_ptr option = find_option(short_name);
        if (!option)
            throw std::invalid_argument("option not found: " + std::string(1, short_name));
        auto result = std::dynamic_pointer_cast<T>(option);
        if (!result)
            throw std::invalid_argument("cannot cast option to T: " + std::string(1, short_name));
        return result;
    }


protected:
    /**
     * @brief Find an option by it's long name.
     * @param long_name
     * @return the option.
     */
    Option_ptr find_option(const std::string& long_name) const;
    
    /**
     * @brief Find an option by it's short name.
     * @param short_name
     * @return the option.
     */
    Option_ptr find_option(char short_name) const;
    
    // Holds the options that have been given.
    std::vector<Option_ptr> options_;
    // Holds a description of the program.
    std::string description_;
    // Any options given after the --
    std::vector<std::string> non_option_args_;
    // Any options that could not be found.
    std::vector<std::string> unknown_options_;
};

/**
 * @class OptionHelp
 * @author rich
 * @date 27/05/21
 * @file Options.h
 * @brief Helper class to format option list.
 */
class OptionHelp
{
public:
    /**
     * @brief Default constructor.
     * @param option_parser
     * @return 
     */
    explicit OptionHelp(const OptionParser* option_parser) :option_parser_(option_parser)
    {
    }

    virtual ~OptionHelp() = default;

    /**
     * @brief return a string of options supported.
     * @param max_attribute
     */
    virtual std::string print() const;
protected:
    /**
     * @brief Pretty print a specific option.
     * @param option
     */
    std::string to_string(Option_ptr option) const;

    const OptionParser* option_parser_;
};

}
