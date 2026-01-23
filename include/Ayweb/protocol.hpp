#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>

namespace ayweb
{

  enum class Method : std::uint8_t
  {
    GET,
    POST,
    PUT,
    DELETE,
    TRACE,
    HEAD,
    OPTIONS,
    CONNECT,
    OTHER,
  };

  struct Request
  {
    Method method;
    std::string path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string content;
  };



  inline std::optional<Request> read_message(const char* data, int n)
  {
    int itr = 0;
    
      }
}  // namespace ayweb
