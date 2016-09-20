#include "spdlog/spdlog.h"

#include <iostream>
#include <memory>

namespace spd = spdlog;
int main(int, char*[])
{
    bool enable_debug = true;
    try
    {
        if( 1 ){

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
            loggers.back()->set_level( spdlog::level::error);

            spdlog::comp_logger comp_logger("composed_logger",  loggers.begin(), loggers.end() );

            comp_logger.warn("This is an info {}", 42);
            comp_logger.info("This is a warning {}", "hello" );
            comp_logger.deb("This is a debug {1} / {0} / {1} ", "second", "first");

        }
        else{

            // This other example use a single logger with multiple sinks.
            // This means that the same log_msg is forwarded to multiple sinks;
            // Each sink can have it's own log level and a message will be logged.
            std::vector<spdlog::sink_ptr> sinks;
            sinks.push_back( std::make_shared<spdlog::sinks::stdout_sink_mt>() );
            sinks.push_back( std::make_shared<spdlog::sinks::simple_file_sink_mt>("./log_regular_file.txt") );
            sinks.push_back( std::make_shared<spdlog::sinks::simple_file_sink_mt>("./log_debug_file.txt") );

            spdlog::logger console_multisink("multisink",  sinks.begin(), sinks.end() );
            console_multisink.set_level( spdlog::level::warn);

            sinks[0]->set_level( spdlog::level::trace);  // console. Allow everything.  Default value
            sinks[1]->set_level( spdlog::level::trace);  //  regular file. Allow everything.  Default value
            sinks[2]->set_level( spdlog::level::off);    //  regular file. Ignore everything.

            console_multisink.warn("warn: will print only on console and regular file");

            if( enable_debug )
            {
                console_multisink.set_level( spdlog::level::debug); // level of the logger
                sinks[1]->set_level( spdlog::level::debug);  // regular file
                sinks[2]->set_level( spdlog::level::debug);  // debug file
            }
            console_multisink.debug("Debug: you should see this on console and both files");
        }

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

