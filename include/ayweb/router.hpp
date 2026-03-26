#pragma once
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tmc/all_headers.hpp>
#include <unordered_map>
#include <utility>

#include "ayweb/export.hpp"
#include "ayweb/protocol.hpp"
#include "tmc/task.hpp"

namespace ayweb
{
  using RouterFun = std::function<tmc::task<Response>(Request)>;
  AYWEB_EXPORT class PathTree
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

    // path: the request path , handle: < Method, Handler>
    void insert(const std::string& path, const std::pair<std::string, RouterFun>& handle);

    // the first is path, the second is method
    std::optional<const RouterFun> search(std::pair<std::string, std::string> pathk);

   private:
    PathNodePtr root;
  };

  AYWEB_EXPORT class Router
  {
   public:
    Router() = default;

    Router(const Router& other) = delete;
    Router& operator=(const Router&) = delete;

    Router(Router&& other) noexcept;
    Router& operator=(Router&& other) noexcept;

    ~Router() = default;

    void route(const std::string& url, const std::string& method, RouterFun&& fun);

    tmc::task<std::optional<Response>> handle(Request req);

   private:
    PathTree tree;
  };
}  // namespace ayweb
