//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/comp_logger.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <memory>
#include <string>


// create logger with given name, sinks and the default pattern formatter
// all other ctors will call this one
template<class It>
inline spdlog::comp_logger::comp_logger(const std::string& logger_name, const It& begin, const It& end):
    logger(logger_name),
    _loggers(begin, end)
{
    _level = level::trace; // delegate the level to the child/children
}

// ctor with sinks as init list
inline spdlog::comp_logger::comp_logger(const std::string& logger_name,  std::initializer_list < logger_ptr > logger_list):
    logger(logger_name),
    _loggers(logger_list.begin(), logger_list.end())
{
    _level = level::trace; // delegate the level to the child/children
}


inline const std::vector<spdlog::logger_ptr>& spdlog::comp_logger::loggers() const
{
    return _loggers;
}

inline const spdlog::logger_ptr& spdlog::comp_logger::operator[] (size_t index) const
{
    return _loggers[index];
}

inline spdlog::comp_logger::~comp_logger() = default;


inline void spdlog::comp_logger::_set_pattern(const std::string& pattern)
{
    for(auto& logger: _loggers)
        logger->spdlog::logger::_set_pattern(pattern);
}

inline void spdlog::comp_logger::_set_formatter(spdlog::formatter_ptr msg_formatter)
{
    for(auto& logger: _loggers)
        logger->spdlog::logger::_set_formatter(msg_formatter);
}

inline void spdlog::comp_logger::_sink_it(details::log_msg& msg)
{
    for(auto& logger: _loggers){

        msg.logger_name = &logger->_name;

        if( logger->should_log(msg.level))
            logger->spdlog::logger::_sink_it(msg);

        msg.formatted.clear();

        if(logger->_should_flush_on(msg))
            logger->spdlog::logger::flush();
    }
}

inline void spdlog::comp_logger::flush()
{
    for(auto& logger: _loggers)
        logger->spdlog::logger::flush();
}


