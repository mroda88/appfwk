/**
 * @file FakeDataConsumerDAQModule.cxx FakeDataConsumerDAQModule class
 * implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "FakeDataConsumerDAQModule.hpp"

#include "TRACE/trace.h"
/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FakeDataConsumer" // NOLINT

#include <chrono>
#include <functional>
#include <thread>

dunedaq::appfwk::FakeDataConsumerDAQModule::FakeDataConsumerDAQModule(const std::string& name)
  : DAQModule(name)
  , queueTimeout_(100)
  , thread_(std::bind(&FakeDataConsumerDAQModule::do_work, this))
  , inputQueue_(nullptr)
{}

void
dunedaq::appfwk::FakeDataConsumerDAQModule::execute_command(const std::string& cmd,
                                                            const std::vector<std::string>& args)
{
  if (cmd == "configure" || cmd == "Configure") {
    do_configure();
  } else if (cmd == "start" || cmd == "Start") {
    do_start();
  } else if (cmd == "stop" || cmd == "Stop") {
    do_stop();
  } else {
    throw UnknownCommand(ERS_HERE, cmd);
  }
}

std::string
dunedaq::appfwk::FakeDataConsumerDAQModule::do_configure()
{
  inputQueue_.reset(new DAQSource<std::vector<int>>(configuration_["input"].get<std::string>()));

  nIntsPerVector_ = configuration_.value<int>("nIntsPerVector", 10);
  starting_int_ = configuration_.value<int>("starting_int", -4);
  ending_int_ = configuration_.value<int>("ending_int", 14);

  return "Success";
}

std::string
dunedaq::appfwk::FakeDataConsumerDAQModule::do_start()
{
  thread_.start_working_thread_();
  return "Success";
}

std::string
dunedaq::appfwk::FakeDataConsumerDAQModule::do_stop()
{
  thread_.stop_working_thread_();
  return "Success";
}

/**
 * @brief Format a std::vector<int> for TRACE
 * @param t TraceStreamer Instance
 * @param ints Vector to format
 * @return TraceStreamer Instance
 */
TraceStreamer&
operator<<(TraceStreamer& t, std::vector<int> ints)
{
  t << "{";
  bool first = true;
  for (auto& i : ints) {
    if (!first)
      t << ", ";
    first = false;
    t << i;
  }
  return t << "}";
}

void
dunedaq::appfwk::FakeDataConsumerDAQModule::do_work()
{
  int current_int = starting_int_;
  int counter = 0;
  int fail_count = 0;
  std::vector<int> vec;

  while (thread_.thread_running()) {
    if (inputQueue_->can_pop()) {

      TLOG(TLVL_DEBUG) << get_name() << ": Going to receive data from inputQueue";

      if (!inputQueue_->pop(vec, queueTimeout_)) {
        TLOG(TLVL_WARNING) << get_name() << ": Tried but failed to pop a value from an inputQueue";
        continue;
      }
      
      TLOG(TLVL_DEBUG) << get_name() << ": Received vector of size "
                       << vec.size();

      bool failed = false;

      TLOG(TLVL_DEBUG) << get_name() << ": Starting processing loop";
      TLOG(TLVL_INFO) << get_name() << ": Received vector " << counter << ": " << vec;
      size_t ii = 0;
      for (auto& point : vec) {
        if (point != current_int) {
          if (ii != 0) {
            TLOG(TLVL_WARNING) << get_name() << ": Error in received vector " << counter << ", position " << ii
                               << ": Expected " << current_int << ", received " << point;
            failed = true;
          } else {
            TLOG(TLVL_INFO) << get_name() << ": Jump detected!";
          }
          current_int = point;
        }
        ++current_int;
        if (current_int > ending_int_)
          current_int = starting_int_;
        ++ii;
      }
      TLOG(TLVL_DEBUG) << get_name() << ": Done with processing loop, failed=" << failed;
      if (failed)
        fail_count++;

      counter++;
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  TLOG(TLVL_INFO) << get_name() << ": Processed " << counter << " vectors with " << fail_count << " failures.";
}

DEFINE_DUNE_DAQ_MODULE(dunedaq::appfwk::FakeDataConsumerDAQModule)
