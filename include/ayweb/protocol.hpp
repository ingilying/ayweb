#pragma once
#include <cctype>
#include <cstddef>
#include <exception>
#include <optional>
#include <ranges>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace ayweb
{

  struct Request
  {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> params;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string content;
  };

  struct Response
  {
    std::string version;
    unsigned int code;
    std::string status;
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

  inline std::pair<std::string, std::unordered_map<std::string, std::string>> parse_params(std::string& url)
  {
    std::unordered_map<std::string, std::string> params;

    std::string_view view(url);
    auto parts = view | std::views::split('?');
    auto itr = parts.begin();
    std::string path((*itr).begin(), (*itr).end());
    std::string_view params_view = (++itr != parts.end()) ? std::string_view((*itr).begin(), (*itr).end()) : "";
    if (!params_view.empty())
    {
      auto pairs = params_view | std::views::split('&');

      for (auto kvs : pairs)
      {
        auto pair = kvs | std::views::split('=');
        auto p_it = pair.begin();
        if (p_it != pair.end())
        {
          std::string key{ (*p_it).begin(), (*p_it).end() };
          std::string value = (++p_it != pair.end()) ? std::string((*p_it).begin(), (*p_it).end()) : "";
          params.insert({ key, value });
        }
      }
    }

    return { std::move(path), std::move(params) };
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
      std::string path;
      std::unordered_map<std::string, std::string> params;
      std::string version;
      try
      {
        head_stream >> method >> url >> version;
        // parase params
        auto [r_path, r_params] = parse_params(url);
        path = std::move(r_path);
        params = std::move(r_params);
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
              header.insert({ key, value });
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
                        .path = std::move(path),
                        .params = std::move(params),
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
