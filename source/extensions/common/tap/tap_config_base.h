#pragma once

#include "envoy/buffer/buffer.h"
#include "envoy/service/tap/v2alpha/common.pb.h"

#include "extensions/common/tap/tap.h"
#include "extensions/common/tap/tap_matcher.h"

namespace Envoy {
namespace Extensions {
namespace Common {
namespace Tap {

/**
 * Common utilities for tapping.
 */
class Utility {
public:
  /**
   * Add body data to a tapped body message, taking into account the maximum bytes to buffer.
   * @param output_body supplies the body message to buffer to.
   * @param max_buffered_bytes supplies the maximum bytes to store, if truncation occurs the
   *        truncation flag will be set.
   * @param data supplies the data to buffer.
   * @param buffer_start_offset supplies the offset within data to start buffering.
   * @param buffer_length_to_copy supplies the length of the data to buffer.
   * @return whether the buffered data was truncated or not.
   */
  static bool addBufferToProtoBytes(envoy::data::tap::v2alpha::Body& output_body,
                                    uint32_t max_buffered_bytes, const Buffer::Instance& data,
                                    uint32_t buffer_start_offset, uint32_t buffer_length_to_copy);

  /**
   * Trim a container that contains buffer raw slices so that the slices start at an offset and
   * only contain a specific length. No slices are removed from the container, but their length
   * may be reduced to 0.
   * TODO(mattklein123): This is split out to ease testing and also because we should ultimately
   * move this directly into the buffer API. I would rather wait until the new buffer code merges
   * before we do that.
   */
  template <typename T> static void trimSlices(T& slices, uint32_t start_offset, uint32_t length) {
    for (auto& slice : slices) {
      const uint32_t start_offset_trim = std::min<uint32_t>(start_offset, slice.len_);
      slice.len_ -= start_offset_trim;
      start_offset -= start_offset_trim;
      slice.mem_ = static_cast<char*>(slice.mem_) + start_offset_trim;

      const uint32_t final_length = std::min<uint32_t>(length, slice.len_);
      slice.len_ = final_length;
      length -= final_length;
    }
  }
};

/**
 * Base class for all tap configurations.
 * TODO(mattklein123): This class will handle common functionality such as rate limiting, etc.
 */
class TapConfigBaseImpl {
public:
  uint32_t maxBufferedRxBytes() const { return max_buffered_rx_bytes_; }
  uint32_t maxBufferedTxBytes() const { return max_buffered_tx_bytes_; }
  size_t numMatchers() const { return matchers_.size(); }
  Matcher& rootMatcher() const;
  void
  submitBufferedTrace(const std::shared_ptr<envoy::data::tap::v2alpha::BufferedTraceWrapper>& trace,
                      uint64_t trace_id);

protected:
  TapConfigBaseImpl(envoy::service::tap::v2alpha::TapConfig&& proto_config,
                    Common::Tap::Sink* admin_streamer);

private:
  // This is the default setting for both RX/TX max buffered bytes. (This means that per tap, the
  // maximum amount that can be buffered is 2x this value).
  static constexpr uint32_t DefaultMaxBufferedBytes = 1024;

  const uint32_t max_buffered_rx_bytes_;
  const uint32_t max_buffered_tx_bytes_;
  Sink* sink_to_use_;
  SinkPtr sink_;
  envoy::service::tap::v2alpha::OutputSink::Format sink_format_;
  std::vector<MatcherPtr> matchers_;
};

/**
 * A tap sink that writes each tap trace to a discrete output file.
 */
class FilePerTapSink : public Sink {
public:
  FilePerTapSink(const envoy::service::tap::v2alpha::FilePerTapSink& config) : config_(config) {}

  // Sink
  void
  submitBufferedTrace(const std::shared_ptr<envoy::data::tap::v2alpha::BufferedTraceWrapper>& trace,
                      envoy::service::tap::v2alpha::OutputSink::Format format,
                      uint64_t trace_id) override;

private:
  const envoy::service::tap::v2alpha::FilePerTapSink config_;
};

} // namespace Tap
} // namespace Common
} // namespace Extensions
} // namespace Envoy
