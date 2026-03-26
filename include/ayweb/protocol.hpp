#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "ayweb/export.hpp"

namespace ayweb
{
  const static unsigned int STATUS_CODE_OK = 200;
  const static unsigned int STATUS_CODE_NOTFOUND = 404;
  class Connection;
  AYWEB_EXPORT struct Request
  {
    std::string_view method;
    std::string_view path;
    std::unordered_map<std::string_view, std::string_view> params;
    std::string_view version;
    std::unordered_map<std::string_view, std::string_view> headers;
    std::string content;
    Connection* conn;
  };

  AYWEB_EXPORT enum class RespOutputOption : std::uint8_t { Normal, Chunked };
  AYWEB_EXPORT struct Response
  {
    std::string version;
    unsigned int code;
    std::string status;
    std::unordered_map<std::string, std::string> headers;
    std::string content;
    RespOutputOption option;
  };

  AYWEB_EXPORT std::string output_response(const Response& resp);
  AYWEB_EXPORT Response response_ok(Request& req);
  AYWEB_EXPORT Response response_notfound(Request& req);
  AYWEB_EXPORT Response build_response(Request& req);

  AYWEB_EXPORT std::optional<std::string_view> read_line(std::string_view& data);

  AYWEB_EXPORT std::string_view trim(std::string_view str);

  AYWEB_EXPORT std::pair<std::string_view, std::unordered_map<std::string_view, std::string_view>> parse_params(
      std::string_view url);
  AYWEB_EXPORT std::optional<Request> read_request(std::string_view data);
  AYWEB_EXPORT std::optional<std::string> read_chunks(std::string_view chunks_str);
}  // namespace ayweb
