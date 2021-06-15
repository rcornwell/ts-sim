#pragma once
#include "Options.h"
#include "ConfigFile.h"

/**
 * @class Application
 * @author rich
 * @date 09/06/21
 * @file Application.h
 * @brief Top level class created to manage a given simulation.
 */
class Application
{
public:
/**
 * @brief Defualt constructor.
 * @return 
 */
    Application()
    {
        auto help_option = op.add<option::OptionSwitch>("h", "help", "produce help message", show_help);
        auto config_option = op.add<option::OptionValue<std::string>("f", "config", "configuration file name", conf_file);
    }

    virtual ~Application()
    {
    }

    bool operator() (int argc, char *argv[])
    {
        try {
            op.parse(argc, argv);
        } catch (invalid_option& inv_opt) {
            std::cout << inv_opt.getValue() << std::endl;
        }
        if (show_help) {
            std::cout << op.help() << std::endl;
            return false;
        }
    }

private:
    core::option::OptionParser    op{"ts-sim options"};
    bool    show_help = false;
    string  conf_file{};
    ConfigFile      cfile;
};
