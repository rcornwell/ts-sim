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

#include "Options.h"

namespace option
{

Option::Option(const std::string& short_name, const std::string &long_name,
               const std::string& description) :
    short_name_(short_name),
    long_name_(long_name),
    description_(description),
    attribute_(Attribute::optional)
{
    if (short_name.size() > 1)
        throw "Argument too long";
    if (short_name.empty() && long_name.empty())
        throw "Need either long or short name";
}

Option_ptr OptionParser::find_option(const std::string& long_name) const
{
    for (const auto& option: options_)
        if (option->getLongName() == long_name)
            return option;
    return nullptr;
}

Option_ptr OptionParser::find_option(char short_name) const
{
    for (const auto& option: options_)
        if (option->getShortName() == short_name)
            return option;
    return nullptr;
}

void OptionParser::parse(int argc, const char * const argv[])
{
    unknown_options_.clear();
    non_option_args_.clear();
    for (auto& opt : options_)
        opt->clear();

    for (int n=1; n<argc; ++n) {
        const std::string arg(argv[n]);
        if (arg == "--") {
            ///from here on only non opt args
            for (int m=n+1; m<argc; ++m)
                non_option_args_.emplace_back(argv[m]);
        } else if (arg.find("--") == 0) {
            /// long option arg
            std::string opt = arg.substr(2);
            std::string optarg;
            size_t equalIdx = opt.find('=');
            if (equalIdx != std::string::npos) {
                optarg = opt.substr(equalIdx + 1);
                opt.resize(equalIdx);
            }

            Option_ptr option = find_option(opt);
            if (option) {
                if (option->getArgument() == Argument::no) {
                    if (!optarg.empty())
                        option = nullptr;
                } else if (option->getArgument() == Argument::required) {
                    if (optarg.empty() && n < argc-1)
                        optarg = argv[++n];
                }
            }

            if (option)
                option->parse(OptionType::longOption, optarg.c_str());
            else
                unknown_options_.push_back(arg);
        } else if (arg.find('-') == 0) {
            /// short option arg
            std::string opt = arg.substr(1);
            bool unknown = false;
            for (size_t m=0; m<opt.size(); ++m) {
                char c = opt[m];
                std::string optarg;

                Option_ptr option = find_option(c);
                if (option) {
                    if (option->getArgument() == Argument::required) {
                        /// use the rest of the current argument as optarg
                        optarg = opt.substr(m + 1);
                        /// or the next arg
                        if (optarg.empty() && n < argc-1)
                            optarg = argv[++n];
                        m = opt.size();
                    } else if (option->getArgument() == Argument::optional) {
                        /// use the rest of the current argument as optarg
                        optarg = opt.substr(m + 1);
                        m = opt.size();
                    }
                }

                if (option)
                    option->parse(OptionType::shortOption, optarg.c_str());
                else
                    unknown = true;
            }
            if (unknown)
                unknown_options_.push_back(arg);
        } else {
            non_option_args_.push_back(arg);
        }
    }

    for (auto& opt : options_) {
        if ((opt->getAttribute() == Attribute::required) && !opt->is_set()) {
            std::string option = opt->getLongName().empty()?std::string(1, opt->getShortName()):opt->getLongName();
            throw invalid_option(opt.get(), invalid_option::Error::missing_option, "option \"" + option + "\" is required");
        }
    }
}

std::string OptionParser::help() const
{
    OptionHelp option_help(this);
    return option_help.print();
}

std::string OptionHelp::print() const
{
    if (option_parser_ == nullptr)
        return "";

    std::stringstream s;
    if (!option_parser_->getDescription().empty())
        s << option_parser_->getDescription() << ":" << std::endl;

    size_t optionRightMargin(20);
    const size_t maxDescriptionLeftMargin(40);

    for (const auto& option: option_parser_->getOptions())
        optionRightMargin = (std::max)(optionRightMargin, to_string(option).size() + 2);
    optionRightMargin = (std::min)(maxDescriptionLeftMargin - 2, optionRightMargin);

    for (const auto& option: option_parser_->getOptions()) {
        if (option->getAttribute() == Attribute::hidden)
            continue;
        std::string optionStr = to_string(option);
        if (optionStr.size() < optionRightMargin)
            optionStr.resize(optionRightMargin, ' ');
        else
            optionStr += '\n' + std::string(optionRightMargin, ' ');
        s << optionStr;

        std::string line;
        std::vector<std::string> lines;
        std::stringstream description(option->getDescription());
        while (std::getline(description, line, '\n'))
            lines.push_back(line);

        std::string empty(optionRightMargin, ' ');
        for (size_t n = 0; n<lines.size(); ++n) {
            if (n > 0)
                s << '\n' << empty;
            s << lines[n];
        }
        s << '\n';

    }
    return s.str();
}

std::string OptionHelp::to_string(Option_ptr option) const
{
    std::stringstream line;
    if (option->getShortName() != 0) {
        line << "  -" << option->getShortName();
        if (!option->getLongName().empty())
            line << ", ";
    } else
        line << "  ";
    if (!option->getLongName().empty())
        line << "--" << option->getLongName();

    if (option->getArgument() == Argument::required) {
        line << " arg";
        std::stringstream defaultStr;
        if (option->getDefault(defaultStr)) {
            if (!defaultStr.str().empty())
                line << " (=" << defaultStr.str() << ")";
        }
    } else if (option->getArgument() == Argument::optional) {
        std::stringstream defaultStr;
        if (option->getDefault(defaultStr))
            line << " [=arg(=" << defaultStr.str() << ")]";
    }

    return line.str();
}

}
