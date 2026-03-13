#pragma once
#include <functional>
#include <memory>
#include <optional>
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

    void insert(const std::string& path, const std::pair<std::string, RouterFun>& handle);

    // the first is path, the second is method
    std::optional<const RouterFun> search(std::pair<std::string, std::string> pathk);

   private:
    PathNodePtr root;
  };
  class Router
  {
   public:
    explicit Router() = default;

    Router(const Router& other) = default;
    Router& operator=(const Router&) = default;

    Router(Router&& other) noexcept;
    Router& operator=(Router&& other) noexcept;

    ~Router() = default;

    void route(std::string url, RouterFun&& fun);

    tmc::task<std::optional<Response>> handle(Request req);

   private:
    std::unordered_map<std::string, RouterFun> reg_map;
  };
}  // namespace ayweb
