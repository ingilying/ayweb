#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <ranges>
#include <string>
#include <tmc/all_headers.hpp>
#include <unordered_map>
#include <utility>

#include "ayweb/protocol.hpp"
#include "tmc/task.hpp"

namespace ayweb
{
  using RouterFun = std::function<tmc::task<Response>(Request)>;
  class PathTree
  {
   private:
    struct PathNode
    {
      std::string path;
      std::unordered_map<std::string, RouterFun> handlers;
      std::unordered_map<std::string, std::unique_ptr<PathNode>> children;
      explicit PathNode(std::string part) : path(std::move(part))
      {
      }
      void set(const std::string& meth, const RouterFun& handle)
      {
        handlers.insert({ meth, handle });
      }
    };
    using PathNodePtr = std::unique_ptr<PathNode>;

   public:
    explicit PathTree() : root(std::make_unique<PathNode>(""))
    {
    }
    PathTree(const PathTree&) = delete;
    PathTree& operator=(const PathTree&) = delete;
    PathTree(PathTree&&) = default;
    PathTree& operator=(PathTree&&) = default;
    ~PathTree() = default;

    void insert(const std::string& path, const std::pair<std::string, RouterFun>& handle)
    {
      auto* current = root.get();

      auto parts = path | std::views::split('/') | std::views::filter([](auto&& ele) { return !ele.empty(); });

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
    
    // the first is path, the second is method
    std::optional<const RouterFun> search(std::pair<std::string, std::string> pathk)
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

   private:
    PathNodePtr root;
  };
  class Router
  {
   public:
    explicit Router() = default;

    Router(const Router& other) = default;
    Router& operator=(const Router&) = default;

    Router(Router&& other) noexcept : reg_map(std::move(other.reg_map))
    {
    }
    Router& operator=(Router&& other) noexcept
    {
      this->reg_map = std::move(other.reg_map);
      return *this;
    }

    ~Router() = default;

    void route(std::string url, RouterFun&& fun)
    {
      reg_map.insert({ std::move(url), std::move(fun) });
    }

    tmc::task<std::optional<Response>> handle(Request req)
    {
      auto fun = reg_map.find(req.path);
      if (fun != reg_map.end())
      {
        auto resp = co_await fun->second(std::move(req));
        co_return resp;
      }
      co_return std::nullopt;
    }

   private:
    std::unordered_map<std::string, RouterFun> reg_map;
  };
}  // namespace ayweb
