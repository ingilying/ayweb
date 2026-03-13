#pragma once
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include "tmc/task.hpp"

namespace ayweb
{
  const static unsigned int STATUS_CODE_OK = 200;
  const static unsigned int STATUS_CODE_NOTFOUND = 404;

  struct Request
  {
    std::string_view method;
    std::string_view path;
    std::unordered_map<std::string_view, std::string_view> params;
    std::string_view version;
    std::unordered_map<std::string_view, std::string_view> headers;
    std::string content;
    std::function<tmc::task<void>()> chunk_progress;
  };

  enum class RespOutputOption : std::uint8_t {
    Normal,
    Chunked
  };

  struct Response
  {
    std::string version;
    unsigned int code;
    std::string status;
    std::unordered_map<std::string, std::string> headers;
    std::string content;
    RespOutputOption option;

  };

  std::string output_response(Response& resp);
  Response response_ok(Request& req);
  Response response_notfound(Request& req);
  Response build_response(Request& req);

  std::optional<std::string_view> read_line(std::string_view& data);

  std::string_view trim(std::string_view str);

  std::pair<std::string_view, std::unordered_map<std::string_view, std::string_view>> parse_params(std::string_view url);
  std::optional<Request> read_request(std::string_view data);
  std::optional<std::string> read_chunks(std::string_view chunks_str);
}  // namespace ayweb
