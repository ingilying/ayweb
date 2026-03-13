#include "ayweb/protocol.hpp"

#include <algorithm>
#include <charconv>
#include <iterator>
#include <optional>
#include <ranges>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include "ayweb/util/func.hpp"

namespace ayweb
{
  std::string output_response(Response& resp)
  {
    std::stringstream sstream;
    sstream << resp.version << " " << resp.code << " " << resp.status << "\r\n";
    for (auto& itr : resp.headers)
    {
      sstream << itr.first << ": " << itr.second << "\r\n";
    }
    sstream << "\r\n";
    sstream << resp.content;
    return sstream.str();
  }
  Response response_ok(Request& req)
  {
    Response resp{ .version = std::string{ req.version }, .code = STATUS_CODE_OK, .status = "OK" };
    return resp;
  }

  Response response_notfound(Request& req)
  {
    Response resp{ .version = std::string{ req.version }, .code = STATUS_CODE_NOTFOUND, .status = "NotFound" };
    return resp;
  }

  Response build_response(Request& req)
  {
    auto resp = response_ok(req);
    return resp;
  }

  std::optional<std::string_view> read_line(std::string_view& data)
  {
    constexpr static std::string_view CRLF = "\r\n";
    auto res = std::ranges::search(data, CRLF);
    if (!res.empty())
    {
      auto line = std::string_view{ data.begin(), res.begin() };
      data = std::string_view{ res.end(), data.end() };
      return line;
    }
    return std::nullopt;
  }

  std::pair<std::string_view, std::unordered_map<std::string_view, std::string_view>> parse_params(std::string_view url)
  {
    std::unordered_map<std::string_view, std::string_view> params;

    auto parts = url | std::views::split('?');
    if (std::distance(parts.begin(), parts.end()) < 2)
    {
      return { url, std::move(params) };
    }
    auto itr = parts.begin();
    std::string_view path(*itr);
    std::string_view params_view = (++itr != parts.end()) ? std::string_view(*itr) : "";
    if (!params_view.empty())
    {
      auto pairs = params_view | std::views::split('&');

      for (auto kvs : pairs)
      {
        auto pair = kvs | std::views::split('=');
        auto p_it = pair.begin();
        if (p_it != pair.end())
        {
          std::string_view key{ *p_it };
          std::string_view value = (++p_it != pair.end()) ? std::string_view{ *p_it } : "";
          params.insert({ key, value });
        }
      }
    }

    return { path, std::move(params) };
  }

  std::string_view trim(std::string_view str)
  {
    auto isspace = [](char chr) { return std::isspace(chr); };

    // skip the front space
    const auto* start = std::ranges::find_if_not(str, isspace);
    if (start == str.end())
    {
      return {};
    }

    // skip the last space
    auto end = std::ranges::find_if_not(str | std::views::reverse, isspace);
    auto btoend = str.rend() - end;
    auto front_distance = std::distance(str.begin(), start);
    return str.substr(front_distance, btoend - front_distance);
  }

  std::optional<Request> read_request(std::string_view data)
  {
    // Request Line
    auto request_line = read_line(data);
    if (request_line != std::nullopt)
    {
      std::unordered_map<std::string_view, std::string_view> params;
      // head_stream >> method >> url >> version;
      auto parts = *request_line | std::views::split(' ');
      if (std::distance(parts.begin(), parts.end()) < 3)
      {
        return std::nullopt;
      }

      auto itr = parts.begin();
      std::string_view method{ *itr };
      ++itr;
      std::string_view url{ *itr };
      ++itr;
      std::string_view version{ *itr };

      // parase params
      auto [r_path, r_params] = parse_params(url);
      auto path = r_path;
      params = std::move(r_params);
      // Header Line
      std::unordered_map<std::string_view, std::string_view> header;
      while (true)
      {
        auto line = read_line(data);
        if (line == std::nullopt || line->empty())
        {
          break;
        }
        auto kvs = (*line) | std::views::split(':');
        auto itc = kvs.begin();
        if (itc != kvs.end())
        {
          auto key = trim(std::string_view{ *itc });
          ++itc;
          if (itc != kvs.end())
          {
            auto value = trim(std::string_view{ *itc });
            header.insert({ key, value });
          }
        }
      }
      return Request{ .method = method,
                      .path = path,
                      .params = params,
                      .version = version,
                      .headers = header,
                      .content = std::move(std::string{}) };
    }
    return std::nullopt;
  }

  std::optional<std::string> read_chunks(std::string_view chunks_str)
  {
    std::vector<std::string_view> chunks;
    int total_size = 0;
    while (true)
    {
      auto len_line = read_line(chunks_str);
      if (!len_line)
      {
        break;
      }
      int chunk_len = -1;
      auto res = std::from_chars(len_line->begin(), len_line->end(), chunk_len);
      if (res.ec != std::errc{})
      {
        break;
      }
      if (chunk_len == 0)
      {
        return utils::accumulate_sv(chunks.begin(), chunks.end(), total_size);
      }
      auto data_line = read_line(chunks_str);
      if (!data_line)
      {
        break;
      }
      if (data_line->size() != chunk_len)
      {
        break;
      }
      total_size += chunk_len;
      chunks.push_back(*data_line);
    }
    return std::nullopt;
  }
}  // namespace ayweb
