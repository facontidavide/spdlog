//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/fmt/fmt.h>

#include <rosbag/bag.h>
#include <ros/this_node.h>
#include <martian_logger/LogMsg.h>

#include <ros/ros.h>
#include <ros/topic_manager.h>
#include <ros/advertise_options.h>
#include <ros/names.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <mutex>
#include <string>
#include <cerrno>

namespace spdlog
{
namespace sinks
{

template<class Mutex>
class rosout_sink : public base_sink < Mutex >
{
public:
  explicit rosout_sink(const std::string& logger_name)
  {
    _log_msg.node_name   = ros::this_node::getName();
    _log_msg.logger_name = logger_name;
  }

  void flush() override {}

protected:

  void _sink_it(const details::log_msg& msg) override
  {
    auto now = ros::Time::now();
    size_t msg_size = msg.formatted.size();
    auto data = msg.formatted.data();

    _log_msg.message = std::string( data, msg_size);
    _log_msg.level = static_cast<uint8_t>( msg.level );
    _log_msg.stamp = now;

    ros::TopicManager::instance()->publish( ros::names::resolve("/rosout2"), _log_msg);

  }

private:
  //const ros::Publisher* _pub;
  martian_logger::LogMsg _log_msg;
};

typedef rosout_sink<std::mutex> rosout_sink_mt;
typedef rosout_sink<details::null_mutex> rosout_sink_st;


}

}
