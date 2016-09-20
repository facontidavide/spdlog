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

            std::vector<std::shared_ptr<spdlog::logger> > consoles;
            consoles.push_back( spd::stdout_logger_mt("toConsole",   true) );
            consoles.push_back( spd::basic_logger_mt("toRegFile",   "./log_regular_file.txt") );
            consoles.push_back( spd::basic_logger_mt("toDebugFile", "./log_debug_file.txt") );
            spdlog::comp_logger console_comp("composed_console",  consoles.begin(), consoles.end() );

            console_comp[0]->set_level( spdlog::level::warn); // console
            console_comp[1]->set_level( spdlog::level::warn); // regular file
            console_comp[2]->set_level( spdlog::level::off); // debug file

            console_comp.warn("warn: will print only on console and regular file");

            if( enable_debug )
            {
                console_comp[0]->set_level( spdlog::level::debug); // console
                console_comp[1]->set_level( spdlog::level::debug); // regular file
                console_comp[2]->set_level( spdlog::level::debug); // debug file
            }
            console_comp.debug("Debug: you should see this on console and both files");

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

