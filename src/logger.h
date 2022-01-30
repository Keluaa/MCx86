#pragma once

#include <iostream>


/**
 * Logger that changes all outputs depending on which mode we are using the computer.
 */
class Logger
{
public:
    enum class Mode
    {
        OFF,
        DEBUG,
        MONITOR_CHANGES
    };

private:
    static Mode mode;
    static std::ostream* null_ostream;

public:
    static void set_mode(Mode new_mode) { mode = new_mode; }
    static Mode get_mode() { return mode; }

    static std::ostream& log()
    {
        switch (mode) {
        case Mode::OFF:
        case Mode::MONITOR_CHANGES:
            return *null_ostream;
        case Mode::DEBUG:
        default:
            return std::cout;
        }
    }

    static std::ostream& err()
    {
        switch (mode) {
        case Mode::OFF:
        case Mode::MONITOR_CHANGES:
        case Mode::DEBUG:
        default:
            return std::cerr;
        }
    }
};
