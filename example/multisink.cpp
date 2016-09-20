#include "spdlog/spdlog.h"

#include <iostream>
#include <memory>

namespace spd = spdlog;
int main(int, char*[])
{
    try
    {
        // this first example shows a composable logger that contains several simple loggers
        // each of these loggers has their own format, level and pattern.

        std::vector<std::shared_ptr<spdlog::logger> > loggers;

        // log to console
        loggers.push_back( spd::stdout_logger_mt("toConsole",   true) );
        loggers.back()->set_level( spdlog::level::info);

        // Create a file rotating logger with 10mb size max and 3 rotated files
        loggers.push_back(spd::rotating_logger_mt("verbose_logger", "logs/log_debug", 1048576 * 10, 5) );
        loggers.back()->set_level( spdlog::level::debug);

        loggers.push_back( spd::basic_logger_mt("error_logger", "./log_error.txt") );
        loggers.back()->set_level( spdlog::level::err);

        spdlog::comp_logger comp_logger("composed_logger",  loggers.begin(), loggers.end() );

        comp_logger.warn("This is an info {}", 42);
        comp_logger.info("This is a warning {}", "hello" );
        comp_logger.debug("This is a debug {1} / {0} / {1} ", "second", "first");

        // Release and close all loggers
        spdlog::drop_all();
    }
    // Exceptions will only be thrown upon failed logger or sink construction (not during logging)
    catch (const spd::spdlog_ex& ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
        return 1;
    }
}

