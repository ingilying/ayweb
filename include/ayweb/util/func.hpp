#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace ayweb::utils
{
  using VECSVITR = std::vector<std::string_view>::iterator;

  inline auto accumulate_sv(VECSVITR first, VECSVITR last, int total_size) -> std::string
  {
    std::string res{};
    res.reserve(total_size);
    for (auto i = first; i != last; ++i)
    {
      res += *i;
    }
    return std::move(res);
  }
}  // namespace ayweb::utils