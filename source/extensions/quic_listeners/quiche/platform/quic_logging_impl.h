#pragma once

// NOLINT(namespace-envoy)

// This file is part of the QUICHE platform implementation, and is not to be
// consumed or referenced directly by other Envoy code. It serves purely as a
// porting layer for QUICHE.

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sstream>

#include "common/common/assert.h"
#include "common/common/logger.h"

#include "absl/base/optimization.h"

// TODO(wub): Add CHECK/DCHECK and variants, which are not explicitly exposed by quic_logging.h.
// TODO(wub): Implement quic_mock_log_impl.h for testing.

// If |condition| is true, use |logstream| to stream the log message and send it to spdlog.
// If |condition| is false, |logstream| will not be instantiated.
// The switch(0) is used to suppress a compiler warning on ambiguous "else".
#define QUIC_LOG_IMPL_INTERNAL(condition, logstream)                                               \
  switch (0)                                                                                       \
  default:                                                                                         \
    if (!(condition)) {                                                                            \
    } else                                                                                         \
      logstream

#define QUIC_LOG_IF_IMPL(severity, condition)                                                      \
  QUIC_LOG_IMPL_INTERNAL((condition) && quic::IsLogLevelEnabled(quic::severity),                   \
                         quic::QuicLogEmitter(quic::severity).stream())

#define QUIC_LOG_IMPL(severity) QUIC_LOG_IF_IMPL(severity, true)

#define QUIC_VLOG_IF_IMPL(verbosity, condition)                                                    \
  QUIC_LOG_IMPL_INTERNAL((condition) && quic::IsVerboseLogEnabled(verbosity),                      \
                         quic::QuicLogEmitter(quic::INFO).stream())

#define QUIC_VLOG_IMPL(verbosity) QUIC_VLOG_IF_IMPL(verbosity, true)

// TODO(wub): Implement QUIC_LOG_FIRST_N_IMPL.
#define QUIC_LOG_FIRST_N_IMPL(severity, n) QUIC_LOG_IMPL(severity)

// TODO(wub): Implement QUIC_LOG_EVERY_N_SEC_IMPL.
#define QUIC_LOG_EVERY_N_SEC_IMPL(severity, seconds) QUIC_LOG_IMPL(severity)

#define QUIC_PLOG_IMPL(severity)                                                                   \
  QUIC_LOG_IMPL_INTERNAL(quic::IsLogLevelEnabled(quic::severity),                                  \
                         quic::QuicLogEmitter(quic::severity).SetPerror().stream())

#define QUIC_LOG_INFO_IS_ON_IMPL quic::IsLogLevelEnabled(quic::INFO)
#define QUIC_LOG_WARNING_IS_ON_IMPL quic::IsLogLevelEnabled(quic::WARNING)
#define QUIC_LOG_ERROR_IS_ON_IMPL quic::IsLogLevelEnabled(quic::ERROR)

#ifdef NDEBUG
// Release build
#define QUIC_COMPILED_OUT_LOG() QUIC_LOG_IMPL_INTERNAL(false, quic::NullLogStream().stream())
#define QUIC_DVLOG_IMPL(verbosity) QUIC_COMPILED_OUT_LOG()
#define QUIC_DVLOG_IF_IMPL(verbosity, condition) QUIC_COMPILED_OUT_LOG()
#define QUIC_DLOG_IMPL(severity) QUIC_COMPILED_OUT_LOG()
#define QUIC_DLOG_IF_IMPL(severity, condition) QUIC_COMPILED_OUT_LOG()
#define QUIC_DLOG_INFO_IS_ON_IMPL 0
#define QUIC_NOTREACHED_IMPL()
#else
// Debug build
#define QUIC_DVLOG_IMPL(verbosity) QUIC_VLOG_IMPL(verbosity)
#define QUIC_DVLOG_IF_IMPL(verbosity, condition) QUIC_VLOG_IF_IMPL(verbosity, condition)
#define QUIC_DLOG_IMPL(severity) QUIC_LOG_IMPL(severity)
#define QUIC_DLOG_IF_IMPL(severity, condition) QUIC_LOG_IF_IMPL(severity, condition)
#define QUIC_DLOG_INFO_IS_ON_IMPL QUIC_LOG_INFO_IS_ON_IMPL
#define QUIC_NOTREACHED_IMPL() NOT_REACHED_GCOVR_EXCL_LINE
#endif

#define QUIC_PREDICT_FALSE_IMPL(x) ABSL_PREDICT_FALSE(x)

namespace quic {

using QuicLogLevel = spdlog::level::level_enum;

static const QuicLogLevel INFO = spdlog::level::info;
static const QuicLogLevel WARNING = spdlog::level::warn;
static const QuicLogLevel ERROR = spdlog::level::err;
static const QuicLogLevel FATAL = spdlog::level::critical;

// DFATAL is FATAL in debug mode, ERROR in release mode.
#ifdef NDEBUG
static const QuicLogLevel DFATAL = ERROR;
#else
static const QuicLogLevel DFATAL = FATAL;
#endif

class QuicLogEmitter {
public:
  explicit QuicLogEmitter(QuicLogLevel level);

  ~QuicLogEmitter();

  QuicLogEmitter& SetPerror() {
    is_perror_ = true;
    return *this;
  }

  std::ostringstream& stream() { return stream_; }

private:
  const QuicLogLevel level_;
  const int saved_errno_;
  bool is_perror_ = false;
  std::ostringstream stream_;
};

class NullLogStream {
public:
  NullLogStream& stream() { return *this; }
};

template <typename T> inline NullLogStream& operator<<(NullLogStream& s, const T&) { return s; }

inline spdlog::logger& GetLogger() {
  return Envoy::Logger::Registry::getLog(Envoy::Logger::Id::quic);
}

inline bool IsLogLevelEnabled(QuicLogLevel level) { return level >= GetLogger().level(); }

int GetVerbosityLogThreshold();
void SetVerbosityLogThreshold(int new_verbosity);

inline bool IsVerboseLogEnabled(int verbosity) {
  return IsLogLevelEnabled(INFO) && verbosity <= GetVerbosityLogThreshold();
}

} // namespace quic
