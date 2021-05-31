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
#include <string>
#include <memory>
#include <cstring>
#include <map>
#include <vector>
#include "ConfigLexer.h"

namespace core
{

class ConfigOptionParser;
enum class Attribute {
    inactive = 0,
    required = 1,
    optional = 2
};


/**
 * @class ConfigOption
 * @author rich
 * @date 25/05/21
 * @file ConfigOption.h
 * @brief Objects of this class are created to represent various options that
 *       a system object might want to know about.
 */
class ConfigOption
{
    friend ConfigOptionParser;

public:
    /**
     * @brief Default constructer.
     * @param name
     * @param description
     */
    ConfigOption(const std::string& name, const std::string& description) :
        name_(name), description_(description),
        attribute_(Attribute::optional)
    {
    }
    virtual ~ConfigOption() = default;

    // Copy Constructor
    ConfigOption(const ConfigOption &) = default;

    // Move Constructor
    ConfigOption(ConfigOption&&) = default;

    // Assignment operator
    ConfigOption& operator=(const ConfigOption&) = default;

    // Move Assignment operator
    ConfigOption& operator=(ConfigOption&&) = default;

    /**
     * @brief Get the name of this option.
     * @return name of option.
     */
    std::string getName() const
    {
        return name_;
    }

    /**
     * @brief Get the desciption associated with this option.
     * @return description.
     */
    std::string getDescription() const
    {
        return description_;
    }

    /**
     * @brief Set the attribute type of this option.
     * @param attribute
     */
    void setAttribute(const Attribute & attribute)
    {
        attribute_ = attribute;
    }

    /**
     * @brief Get the atttribute type of this option.
     * @return Attribute value.
     */
    Attribute getAttribute() const
    {
        return attribute_;
    }

    /**
     * @brief Indicate if this option has been seen.
     * @return bool
     */
    virtual bool is_set() const = 0;

protected:
    /**
     * @brief Scan the configuration file and set this option.
     * @param p_lexer
     */
    virtual void parse(ConfigLexer *p_lexer) = 0;

    /**
     * @brief Indicate that his option is no longer set.
     */
    virtual void clear() = 0;

    /**
     * @brief Name of option.
     */
    std::string name_;

    /**
     * @brief Description of option for help.
     */
    std::string description_;

    /**
     * @brief Attribute of this option.
     */
    Attribute attribute_;
};


/**
 * @class ConfigValue
 * @author rich
 * @date 25/05/21
 * @file ConfigOption.h
 * @brief Generic option class, handles most options.
 */
template<class T>
class ConfigValue : public ConfigOption
{
public:
    /**
     * @brief Default constructor.
     * @param name - option name.
     * @param description - description of options use.
     * @detail Generally not used.
     */
    ConfigValue(const std::string& name, const std::string& description):
        ConfigOption(name, description), assign_to_(nullptr)
    {
        is_set_ = false;
    }

    /**
     * @brief Normal constructor.
     * @param name - option name.
     * @param description - description of options use.
     * @param default_val - default value to assign if not seen in options.
     * @param assign_to - location to set.
     */
    ConfigValue(const std::string& name, const std::string& description,
                const T& default_val, T* assign_to = nullptr):
        ConfigValue<T>(name, description)
    {
        assign_to_ = assign_to;
        is_set_ = false;
        setDefault(default_val);
    }

    /**
     * @brief Indicate if this option has been seen.
     * @return bool
     */
    bool is_set() const override
    {
        return is_set_;
    }

    /**
     * @brief Sets location to assign values too.
     * @param var Location of variable in object.
     */
    void assign_to(T* var)
    {
        assign_to_ = var;
        update_reference();
    }

    /**
     * @brief Set the value to this value.
     * @param value - Value to set.
     */
    void setValue(const T& value)
    {
        clear();
        add_value(value);
    }

    /**
     * @brief Get the value of the option.
     * @return value of object.
     */
    T getValue() const
    {
        // If not set and default, return default
        if (!this->is_set() && default_) {
            return *default_;
        }

        // If not set and no default, throw exception.
        if (!this->is_set()) {
            throw "";
        }
        return value_;
    }

    /**
     * @brief Set the default value to use if option not seen.
     * @param value
     */
    void setDefault(const T& value)
    {
        this->default_.reset(new T);
        *this->default_ = value;
        update_reference();
    }

    /**
     * @brief Indicate if a default value is set.
     * @return value
     */
    bool hasDefault() const
    {
        return (this->default_ != nullptr);
    }

    /**
     * @brief Get the default value or nullptr if not set.
     * @return value
     */
    T getDefault() const
    {
        return *this->default_;
    }

protected:
    /**
     * @brief Parse this option. Must be overridden for each supported type.
     * @param p_lexer
     */
    void parse(ConfigLexer *p_lexer) override;

    /**
     * @brief Called after a change is made to option to update default and
     *      assign values.
     */
    virtual void update_reference()
    {
        if (this->assign_to_) {
            if (this->is_set() || default_)
                *this->assign_to_ = getValue();
        }
    }


    /**
     * @brief Add a value to option. Currently only one value is supported.
     * @param value
     */
    virtual void add_value(const T& value)
    {
        value_ = value;
        is_set_ = true;
        update_reference();
    }

    /**
     * @brief restore option to being unset.
     */
    void clear() override
    {
        is_set_ = false;
        update_reference();
    }

    /**
     * @brief Pointer to default value.
     */
    std::unique_ptr<T> default_;

    /**
     * @brief Point to where to set when option changed.
     */
    T* assign_to_;

    /**
     * @brief Current value of option.
     */
    T  value_{};

    /**
     * @brief flag to indicate that option has been set.
     */
    bool is_set_;
};


/**
 * @class ConfigBool
 * @author rich
 * @date 25/05/21
 * @file ConfigOption.h
 * @brief Simple shortcut for declaring flags. The default is not set.
 */
class ConfigBool : public ConfigValue<bool>
{
public:
    /**
     * @brief Default constructor.
     * @param name - Name of option
     * @param description - Description of option
     * @param assign_to - Location to set on change.
     */
    ConfigBool(const std::string& name, const std::string& description,
               bool* assign_to = nullptr): ConfigValue<bool>(name, description, false, assign_to)
    {
        is_set_ = false;
    }

    // The default value for boolean flags is false.
    void setDefault(const bool& value) = delete;
protected:
    /**
     * @brief Parse this option. Must be overridden for each supported type.
     * @param p_lexer
     */
    void parse(ConfigLexer *p_lexer) override;
};


/**
 * @class ConfigMap
 * @author rich
 * @date 25/05/21
 * @file ConfigOption.h
 * @brief This is a multi-choice option. Call add_option to add in values.
 */
template<class T>
class ConfigMap : public ConfigValue<T>
{
public:
    /**
      * @brief Normal constructor.
      * @param name - option name.
      * @param description - description of options use.
      * @param default_val - default value to assign if not seen in options.
      * @param assign_to - location to set.
      */
    ConfigMap(const std::string& name, const std::string& description,
              const T& default_val, T* assign_to = nullptr) :
        ConfigValue<T>(name, description, default_val, assign_to)
    {
    }

    /**
     * @brief Add a multiple choice option setting.
     * @param value - Value to set.
     * @param name - Name of option.
     */
    void add_option(T& value, const std::string& name)
    {
        value_map_.insert(name, value);
    }

protected:
    /**
     * @brief Parse this option. Must be overridden for each supported type.
     * @param p_lexer
     */
    void parse(ConfigLexer *p_lexer) override;

    /**
    * @brief Called after a change is made to option to update default and
    *      assign values.
    */
    virtual void update_reference() override;

    /**
     * @brief Add a value to option. Currently only one value is supported.
     * @param value
     */
    virtual void add_value(const std::string& value) override;

    /**
     * @brief Map of possible choices.
     */
    std::map<std::string, T> value_map_;
};

using ConfigOption_ptr = std::shared_ptr<ConfigOption>;

/**
 * @class ConfigOptionParser
 * @author rich
 * @date 25/05/21
 * @file ConfigOption.h
 * @brief This object holds the options and calls each to set the value.
 */
class ConfigOptionParser
{
public:
    /**
     * @brief Default constructor.
     */
    explicit ConfigOptionParser(std::string description = "");

    virtual ~ConfigOptionParser() = default;

    /**
     * @brief Get the desciption associated with this option.
     * @return description.
     */
    std::string getDescription() const
    {
        return description_;
    }

    /**
    * @brief Add an option with a given attribute.
    * @return  pointer to option.
    */
    template<typename T, Attribute attribute, typename... Ts>
    std::shared_ptr<T> add(Ts&&... params)
    {
        static_assert(
            std::is_base_of<ConfigOption, typename std::decay<T>::type>::value,
            "type T must be Bool, Value or Implicit"
        );
        std::shared_ptr<T> option = std::make_shared<T>(std::forward<Ts>(params)...);

        for (const auto& o: options_) {
            if (option->getName() == o->getName())
                throw std::invalid_argument("duplicate option" + option->getName());
        }
        option->setAttribute(attribute);
        options_.push_back(option);
        return option;
    }


    /**
    * @brief Add an option with a default attribute.
    * @return  pointer to option.
    */
    template<typename T, typename... Ts>
    std::shared_ptr<T> add(Ts&&... params)
    {
        return add<T, Attribute::optional>(std::forward<Ts>(params)...);
    }


    /**
     * @brief Scan and set a collection of options.
     * @param p_lexer
     */
    void parse(core::ConfigLexer*  p_lexer);

    /**
     * @brief Get all options added.
     * @return Vector of options.
     */
    const std::vector<ConfigOption_ptr>& getOptions() const;

protected:

    /**
     * @brief Find a option based on name.
     * @param name - name of option
     * @return Pointer to option.
     */
    ConfigOption_ptr find_option(const std::string& name) const;

    /**
     * Description of these arguments.
     */
    std::string description_;
    /**
     * Vector of options.
     */
    std::vector<ConfigOption_ptr> options_;
};

}
