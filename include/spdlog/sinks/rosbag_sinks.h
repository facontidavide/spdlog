//
// Copyright(c) 2015 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/details/file_helper.h>
#include <spdlog/fmt/fmt.h>

#include <rosbag/bag.h>
#include <ros/this_node.h>
#include <martian_logger/LogMsg.h>

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

class rosbag_helper
{

public:

  explicit rosbag_helper(){ }

  rosbag_helper(const rosbag_helper&) = delete;
  rosbag_helper& operator=(const rosbag_helper&) = delete;

  ~rosbag_helper() {
    close();
  }

  void open(const filename_t& filename, bool truncate = true)
  {
    _filename = filename;

    if(truncate)
      _bag.open( filename, rosbag::bagmode::Write);
    else
      _bag.open( filename, rosbag::bagmode::Append);


    _bag.setCompression( rosbag::CompressionType::LZ4 );
  }

  void close()
  {
    _bag.close();
  }

  void write(const martian_logger::LogMsg& log_msg)
  {
    _bag.write("rosout2", log_msg.stamp, log_msg);
  }

  void reopen(bool truncate)
  {
    if (_filename.empty())
      throw spdlog_ex("Failed re opening file - was not opened before");
    open(_filename, truncate);

  }

private:
  rosbag::Bag _bag;
  filename_t _filename;
};



template<class Mutex>
class rosbag_sink : public base_sink < Mutex >
{
public:
  explicit rosbag_sink(const std::string& logger_name, const filename_t &filename, bool truncate = false)
  {
    _log_msg.node_name   = ros::this_node::getName();
    _log_msg.logger_name = logger_name;
    _bag_helper.open(filename, truncate);
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

    _bag_helper.write(_log_msg);
  }

private:
  rosbag_helper _bag_helper;
  martian_logger::LogMsg _log_msg;
};

typedef rosbag_sink<std::mutex> rosbag_sink_mt;
typedef rosbag_sink<details::null_mutex> rosbag_sink_st;

//-------------------------------------------------------------

/*
 * Rotating file sink based on size
 */
template<class Mutex>
class rotating_rosbag_sink : public base_sink < Mutex >
{
public:
  rotating_rosbag_sink(const std::string& logger_name,
                       const filename_t &base_filename, const filename_t &extension,
                       std::size_t max_size, std::size_t max_files ) :
    _base_filename(base_filename),
    _extension(extension),
    _max_size(max_size),
    _max_files(max_files),
    _current_size(0)
  {
    _log_msg.node_name   = ros::this_node::getName();
    _log_msg.logger_name = logger_name;
    _bag_helper.open(calc_filename(_base_filename, 0, _extension));
  }

  void flush() override{ }

protected:
  void _sink_it(const details::log_msg& msg) override
  {
    auto now = ros::Time::now();
    size_t msg_size = msg.formatted.size();
    auto data = msg.formatted.data();

    _log_msg.message = std::string( data, msg_size);
    _log_msg.level = static_cast<uint8_t>( msg.level );
    _log_msg.stamp = now;

    _current_size += _log_msg.message.size() + _log_msg.node_name.size() +  _log_msg.logger_name.size() + sizeof(now);
    if (_current_size > _max_size)
    {
      _rotate();
      _current_size = msg.formatted.size();
    }
    _bag_helper.write(_log_msg);
  }

private:
  static filename_t calc_filename(const filename_t& filename, std::size_t index, const filename_t& extension)
  {
    std::conditional<std::is_same<filename_t::value_type, char>::value, fmt::MemoryWriter, fmt::WMemoryWriter>::type w;
    if (index)
      w.write(SPDLOG_FILENAME_T("{}.{}.{}"), filename, index, extension);
    else
      w.write(SPDLOG_FILENAME_T("{}.{}"), filename, extension);
    return w.str();
  }

  // Rotate files:
  // log.txt -> log.1.txt
  // log.1.txt -> log2.txt
  // log.2.txt -> log3.txt
  // log.3.txt -> delete

  void _rotate()
  {
    using details::os::filename_to_str;
    _bag_helper.close();
    for (auto i = _max_files; i > 0; --i)
    {
      filename_t src = calc_filename(_base_filename, i - 1, _extension);
      filename_t target = calc_filename(_base_filename, i, _extension);

      if (details::file_helper::file_exists(target))
      {
        if (details::os::remove(target) != 0)
        {
          throw spdlog_ex("rotating_rosbag_sink: failed removing " + filename_to_str(target), errno);
        }
      }
      if (details::file_helper::file_exists(src) && details::os::rename(src, target))
      {
        throw spdlog_ex("rotating_rosbag_sink: failed renaming " + filename_to_str(src) + " to " + filename_to_str(target), errno);
      }
    }
    _bag_helper.reopen(true);
  }

  filename_t _base_filename;
  filename_t _extension;
  std::size_t _max_size;
  std::size_t _max_files;
  std::size_t _current_size;
  rosbag_helper _bag_helper;
  martian_logger::LogMsg _log_msg;
};

typedef rotating_rosbag_sink<std::mutex> rotating_rosbag_sink_mt;
typedef rotating_rosbag_sink<details::null_mutex>rotating_rosbag_sink_st;


}
}
