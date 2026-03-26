#include "ayweb/router.hpp"

#include <ranges>

#include "ayweb/protocol.hpp"

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
  Router::Router(Router&& other) noexcept : tree(std::move(other.tree))
  {
  }

  Router& Router::operator=(Router&& other) noexcept
  {
    this->tree = std::move(other.tree);
    return *this;
  }

  void Router::route(const std::string& url, const std::string& method, RouterFun&& fun)
  {
    tree.insert(url, { method, std::move(fun) });
  }

  tmc::task<std::optional<Response>> Router::handle(Request req)
  {
    auto fun = tree.search({ std::string{ req.path }, std::string{ req.method } });
    if (fun.has_value())
    {
      auto resp = co_await (*fun)(std::move(req));
      co_return resp;
    }
    else
    {
      co_return response_notfound(req);
    }
  }
}  // namespace ayweb
