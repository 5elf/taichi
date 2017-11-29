/*******************************************************************************
    Taichi - Physically based Computer Graphics Library

    Copyright (c) 2017 Yuanming Hu <yuanmhu@gmail.com>

    All rights reserved. Use of this source code is governed by
    the MIT license as written in the LICENSE file.
*******************************************************************************/

#include <taichi/util.h>
#include <taichi/system/threading.h>
#include <csignal>
#include <spdlog/spdlog.h>

TC_NAMESPACE_BEGIN

void signal_handler(int signo);

#define TC_REGISTER_SIGNAL_HANDLER(name, handler)                    \
  {                                                                  \
    if (std::signal(name, handler) == SIG_ERR)                       \
      std::printf("Can not register signal handler for" #name "\n"); \
  }

Logger::Logger() {
  console = spdlog::stdout_color_mt("console");
  TC_LOG_SET_PATTERN("[%L %D %X.%e] %v")

  TC_REGISTER_SIGNAL_HANDLER(SIGSEGV, signal_handler);
  TC_REGISTER_SIGNAL_HANDLER(SIGABRT, signal_handler);
  TC_REGISTER_SIGNAL_HANDLER(SIGBUS, signal_handler);
  TC_REGISTER_SIGNAL_HANDLER(SIGFPE, signal_handler);
  spdlog::set_level(spdlog::level::trace);
  TC_TRACE("Taichi core started. Thread ID = {}", PID::get_pid());
}

void Logger::trace(const std::string &s) {
  console->trace(s);
}

void Logger::debug(const std::string &s) {
  console->debug(s);
}

void Logger::info(const std::string &s) {
  console->info(s);
}

void Logger::warn(const std::string &s) {
  console->warn(s);
}
void Logger::error(const std::string &s, bool raise_signal) {
  console->error(s);
  if (raise_signal) {
    std::raise(SIGABRT);
  }
}
void Logger::critical(const std::string &s, bool raise_signal) {
  console->critical(s);
  if (raise_signal) {
    std::raise(SIGABRT);
  }
}
void Logger::flush() {
  console->flush();
}

Logger logger;

void signal_handler(int signo) {
  logger.error(fmt::format("Received signal {} ({})", signo, strsignal(signo)),
               false);
  TC_FLUSH_LOGGER;
  taichi::print_traceback();
  if (taichi::CoreState::get_instance().python_imported) {
    std::string msg =
        fmt::format("Taichi Core Exception: {} ({})", signo, strsignal(signo));
    taichi_raise_assertion_failure_in_python(msg.c_str());
  }
  std::exit(-1);
}

TC_NAMESPACE_END
