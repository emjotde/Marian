#pragma once

#include <iostream>

#include "spdlog/spdlog.h"

namespace marian {
  void logCallStack(size_t skipLevels);
}

/**
 * Prints logging message into stderr and a file specified with `--log` option.
 *
 * Example usage: `LOG(info, "[data] Vocab size: {}", vocabSize)`
 *
 * A good practice is to put `[namespace]` at the beginning of the message.
 *
 * @param level Logging level: trace, debug, info, warn, error, critical
 * @param ... Message text and variables
 */
#define LOG(level, ...) checkedLog("general", #level, __VA_ARGS__)

/**
 * Prints logging message regarding validation into stderr and a file specified
 * with `--valid-log` option.
 *
 * The message is automatically preceded by "[valid] ".
 *
 * @see \def LOG(level, ...)
 */
#define LOG_VALID(level, ...) checkedLog("valid", #level, __VA_ARGS__)

#ifdef __GNUC__
#define FUNCTION_NAME __PRETTY_FUNCTION__
#else
#ifdef _WIN32
#define FUNCTION_NAME __FUNCTION__
#else
#define FUNCTION_NAME "???"
#endif
#endif

/**
 * Prints critical error message and causes abnormal program termination by
 * calling std::abort().
 *
 * @param ... Message text and variables
 */
#define ABORT(...)                                                      \
  do {                                                                  \
    checkedLog("general", "critical", __VA_ARGS__);                     \
    ::marian::logCallStack(/*skipLevels=*/0);                           \
    std::cerr << "Aborted from " << FUNCTION_NAME << " in " << __FILE__ \
              << ": " << __LINE__ << std::endl;                         \
    abort();                                                            \
  } while(0)

/**
 * Prints critical error message and causes abnormal program termination if
 * conditions is true.
 *
 * @param condition Condition expression
 * @param ... Message text and variables
 *
 * @see \def ABORT(...)
 */
#define ABORT_IF(condition, ...) \
  do {                           \
    if(condition) {              \
      ABORT(__VA_ARGS__);        \
    }                            \
  } while(0)

typedef std::shared_ptr<spdlog::logger> Logger;
Logger createStderrLogger(const std::string&,
                          const std::string&,
                          const std::vector<std::string>& = {},
                          bool quiet = false);

namespace marian {
class Config;
}

template <class... Args>
void checkedLog(std::string logger, std::string level, Args... args) {
  Logger log = spdlog::get(logger);
  if(!log) {
    if(level == "critical") {
      // log and errlog are not the same, hence we need to check
      // if an error logger exists first and not try to create a
      // second one. Otherwise this will throw an exception.
      Logger errlog = spdlog::get("error");
      if(!errlog)
        errlog = createStderrLogger("error", "Error: %v - aborting");
      errlog->critical(args...);
    }
    // @TODO: should other loggers do something? This seems to be
    // a sink state when logs are not intialized. Critical errors
    // should log nevertheless, non-critical go unreported.
    return;
  }

  if(level == "trace")
    log->trace(args...);
  else if(level == "debug")
    log->debug(args...);
  else if(level == "info")
    log->info(args...);
  else if(level == "warn")
    log->warn(args...);
  else if(level == "error")
    log->error(args...);
  else if(level == "critical")
    log->critical(args...);
  else {
    log->warn("Unknown log level '{}' for logger '{}'", level, logger);
  }
}

void createLoggers(const marian::Config* options = nullptr);
void switchtoMultinodeLogging(std::string nodeIdStr);
