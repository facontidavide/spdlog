#ifndef LOGGER_COMPOSABLE_H
#define LOGGER_COMPOSABLE_H

namespace spdlog
{

using logger_ptr = std::shared_ptr < logger >;

class comp_logger: public logger
{
public:
    comp_logger(const std::string& logger_name, std::initializer_list < logger_ptr >);

    template<class It>
    comp_logger(const std::string& name, const It& begin, const It& end);

    virtual ~comp_logger();

    const std::vector<logger_ptr> &loggers() const;

    const logger_ptr& operator[] (size_t index) const;

    // error handler of ALL the loggers
    void set_error_handler(log_err_handler);
    log_err_handler error_handler();

    void flush() override;

protected:

    virtual void _sink_it(details::log_msg& msg) override;
    virtual void _set_formatter(spdlog::formatter_ptr msg_formatter) override;
    virtual void _set_pattern(const std::string& pattern) override;

    std::vector<logger_ptr> _loggers;
    std::atomic<time_t> _last_err_time;
};
}

#include <spdlog/details/comp_logger_impl.h>
#endif
