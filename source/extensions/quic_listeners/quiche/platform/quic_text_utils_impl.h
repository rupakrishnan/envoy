#pragma once

#include "absl/strings/ascii.h"
#include "absl/strings/escaping.h"
#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include "quiche/quic/platform/api/quic_string.h"
#include "quiche/quic/platform/api/quic_string_piece.h"

// NOLINT(namespace-envoy)

// This file is part of the QUICHE platform implementation, and is not to be
// consumed or referenced directly by other Envoy code. It serves purely as a
// porting layer for QUICHE.

namespace quic {

class QuicTextUtilsImpl {
public:
  static bool StartsWith(QuicStringPiece data, QuicStringPiece prefix) {
    return absl::StartsWith(data, prefix);
  }

  static bool EndsWithIgnoreCase(QuicStringPiece data, QuicStringPiece suffix) {
    return absl::EndsWithIgnoreCase(data, suffix);
  }

  static QuicString ToLower(QuicStringPiece data) { return absl::AsciiStrToLower(data); }

  static void RemoveLeadingAndTrailingWhitespace(QuicStringPiece* data) {
    *data = absl::StripAsciiWhitespace(*data);
  }

  static bool StringToUint64(QuicStringPiece in, uint64_t* out) {
    return absl::SimpleAtoi(in, out);
  }

  static bool StringToInt(QuicStringPiece in, int* out) { return absl::SimpleAtoi(in, out); }

  static bool StringToUint32(QuicStringPiece in, uint32_t* out) {
    return absl::SimpleAtoi(in, out);
  }

  static bool StringToSizeT(QuicStringPiece in, size_t* out) { return absl::SimpleAtoi(in, out); }

  static QuicString Uint64ToString(uint64_t in) { return absl::StrCat(in); }

  static QuicString HexEncode(QuicStringPiece data) { return absl::BytesToHexString(data); }

  static QuicString Hex(uint32_t v) { return absl::StrCat(absl::Hex(v)); }

  static QuicString HexDecode(QuicStringPiece data) { return absl::HexStringToBytes(data); }

  static void Base64Encode(const uint8_t* data, size_t data_len, QuicString* output);

  static QuicString HexDump(QuicStringPiece binary_data);

  static bool ContainsUpperCase(QuicStringPiece data) {
    return std::any_of(data.begin(), data.end(), absl::ascii_isupper);
  }

  static std::vector<QuicStringPiece> Split(QuicStringPiece data, char delim) {
    return absl::StrSplit(data, delim);
  }
};

} // namespace quic
