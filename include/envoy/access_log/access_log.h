#pragma once

#include <memory>
#include <string>

#include "envoy/common/pure.h"
#include "envoy/filesystem/filesystem.h"
#include "envoy/http/header_map.h"
#include "envoy/stream_info/stream_info.h"

namespace Envoy {
namespace AccessLog {

class AccessLogManager {
public:
  virtual ~AccessLogManager() {}

  /**
   * Reopen all of the access log files.
   */
  virtual void reopen() PURE;

  /**
   * Create a new access log file managed by the access log manager.
   * @param file_name specifies the file to create/open.
   * @return the opened file.
   */
  virtual Filesystem::FileSharedPtr createAccessLog(const std::string& file_name) PURE;
};

typedef std::unique_ptr<AccessLogManager> AccessLogManagerPtr;

/**
 * Interface for access log filters.
 */
class Filter {
public:
  virtual ~Filter() {}

  /**
   * Evaluate whether an access log should be written based on request and response data.
   * @return TRUE if the log should be written.
   */
  virtual bool evaluate(const StreamInfo::StreamInfo& info, const Http::HeaderMap& request_headers,
                        const Http::HeaderMap& response_headers,
                        const Http::HeaderMap& response_trailers) PURE;
};

typedef std::unique_ptr<Filter> FilterPtr;

/**
 * Abstract access logger for requests and connections.
 */
class Instance {
public:
  virtual ~Instance() {}

  /**
   * Log a completed request.
   * @param request_headers supplies the incoming request headers after filtering.
   * @param response_headers supplies response headers.
   * @param response_trailers supplies response trailers.
   * @param stream_info supplies additional information about the request not
   * contained in the request headers.
   */
  virtual void log(const Http::HeaderMap* request_headers, const Http::HeaderMap* response_headers,
                   const Http::HeaderMap* response_trailers,
                   const StreamInfo::StreamInfo& stream_info) PURE;
};

typedef std::shared_ptr<Instance> InstanceSharedPtr;

/**
 * Interface for access log formatter.
 * Formatters provide a complete access log output line for the given headers/trailers/stream.
 */
class Formatter {
public:
  virtual ~Formatter() {}

  /**
   * Return a formatted access log line.
   * @param request_headers supplies the request headers.
   * @param response_headers supplies the response headers.
   * @param response_trailers supplies the response trailers.
   * @param stream_info supplies the stream info.
   * @return std::string string containing the complete formatted access log line.
   */
  virtual std::string format(const Http::HeaderMap& request_headers,
                             const Http::HeaderMap& response_headers,
                             const Http::HeaderMap& response_trailers,
                             const StreamInfo::StreamInfo& stream_info) const PURE;
};

typedef std::unique_ptr<Formatter> FormatterPtr;

/**
 * Interface for access log provider.
 * FormatterProviders extract information from the given headers/trailers/stream.
 */
class FormatterProvider {
public:
  virtual ~FormatterProvider() {}

  /**
   * Extract a value from the provided headers/trailers/stream.
   * @param request_headers supplies the request headers.
   * @param response_headers supplies the response headers.
   * @param response_trailers supplies the response trailers.
   * @param stream_info supplies the stream info.
   * @return std::string containing a single value extracted from the given headers/trailers/stream.
   */
  virtual std::string format(const Http::HeaderMap& request_headers,
                             const Http::HeaderMap& response_headers,
                             const Http::HeaderMap& response_trailers,
                             const StreamInfo::StreamInfo& stream_info) const PURE;
};

typedef std::unique_ptr<FormatterProvider> FormatterProviderPtr;

} // namespace AccessLog
} // namespace Envoy
