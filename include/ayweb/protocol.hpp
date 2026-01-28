#pragma once
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <optional>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

namespace ayweb
{

  struct Request
  {
    std::string method;
    std::string path;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string content;
  };

  inline std::optional<std::string> read_line(std::span<const char>& data, std::span<const char>::iterator& itr)
  {
    auto begin = itr;
    while (itr != data.end())
    {
      if (*itr == '\r')
      {
        itr++;
        if (itr != data.end())
        {
          if (*itr == '\n')
          {
            auto end = itr - 1;
            itr++;
            return std::string(begin, end);
          }
        }
      }
      else
      {
        itr++;
      }
    }
    return std::nullopt;
  }

  inline auto trim(auto&& range)
  {
    auto isspace = [](unsigned char ccc) { return std::isspace(ccc); };
    return range | std::views::drop_while(isspace) | std::views::reverse | std::views::drop_while(isspace) |
           std::views::reverse;
  }

  inline std::optional<Request> read_message(const char* data, std::size_t n)
  {
    std::span<const char> data_view{ data, n };
    auto itr = data_view.begin();
    // Request Line
    auto request_line = read_line(data_view, itr);
    if (request_line != std::nullopt)
    {
      std::stringstream head_stream(*request_line);
      std::string method;
      std::string url;
      std::string version;
      try
      {
        head_stream >> method >> url >> version;

        // Header Line
        std::unordered_map<std::string, std::string> header;
        while (true)
        {
          auto line = read_line(data_view, itr);
          if (line == std::nullopt || line->empty())
          {
            break;
          }
          auto kvs = (*line) | std::views::split(':');
          auto itc = kvs.begin();
          if (itc != kvs.end())
          {
            auto key_view = trim(*itc);
            std::string key{ key_view.begin(), key_view.end() };
            ++itc;
            if (itc != kvs.end())
            {
              auto value_view = trim(*itc);
              std::string value{ value_view.begin(), value_view.end() };
              header.insert({key, value});
            }
          }
        }
        // Content
        ++itr;
        std::string content;
        if (itr != data_view.end())
        {
          content = std::string(itr, data_view.end());
        }
        return Request{ .method = std::move(method),
                        .path = std::move(url),
                        .version = std::move(version),
                        .headers = std::move(header),
                        .content = std::move(content) };
      }
      catch (std::exception& ep)
      {
        return std::nullopt;
      }
    }
    return std::nullopt;
  }
}  // namespace ayweb
