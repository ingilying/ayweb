#include "ayweb/router.hpp"
#include <ranges>

namespace ayweb
{
  // PathTree implementations
  void PathTree::insert(const std::string& path, const std::pair<std::string, RouterFun>& handle)
  {
    auto* current = root.get();

    auto parts = path | std::ranges::views::split('/') | std::views::filter([](auto&& ele) { return !ele.empty(); });

    for (const auto& part : parts)
    {
      std::string key{ part.begin(), part.end() };
      if (!current->children.contains(key))
      {
        current->children.insert({ key, std::make_unique<PathNode>(key) });
      }
      current = current->children[key].get();
    }
    current->set(handle.first, handle.second);
  }

  std::optional<const RouterFun> PathTree::search(std::pair<std::string, std::string> pathk)
  {
    auto* current = root.get();

    auto parts = pathk.first | std::views::split('/') | std::views::filter([](auto&& ele) { return !ele.empty(); });

    for (const auto& part : parts)
    {
      std::string key{ part.begin(), part.end() };
      if (current->children.contains(key))
      {
        current = current->children[key].get();
      }
      else
      {
        return {};
      }
    }
    auto res = current->handlers.find(pathk.second);
    if (res != current->handlers.end())
    {
      return { res->second };
    }
    return {};
  }

  // Router implementations
  Router::Router(Router&& other) noexcept : reg_map(std::move(other.reg_map))
  {
  }

  Router& Router::operator=(Router&& other) noexcept
  {
    this->reg_map = std::move(other.reg_map);
    return *this;
  }

  void Router::route(std::string url, RouterFun&& fun)
  {
    reg_map.insert({ std::move(url), std::move(fun) });
  }

  tmc::task<std::optional<Response>> Router::handle(Request req)
  {
    auto fun = reg_map.find(std::string{req.path});
    if (fun != reg_map.end())
    {
      auto resp = co_await fun->second(std::move(req));
      co_return resp;
    }
    co_return std::nullopt;
  }
}  // namespace ayweb
