#include <asio/ip/address.hpp>
#include <asio/ip/address_v4.hpp>
#include <iostream>

#include "ayweb/http_server.hpp"
#include "ayweb/protocol.hpp"
#include "ayweb/router.hpp"
#include "tmc/task.hpp"

int main()
{
  std::cout << "Example Echo Server" << '\n';
  constexpr unsigned int PORT = 8888;
  ayweb::HttpServer server(asio::ip::address_v4::any(), PORT);
  ayweb::Router router;
  router.route(
      "/",
      "GET",
      [](ayweb::Request req) -> tmc::task<ayweb::Response>  // NOLINT
      {
        auto resp = ayweb::response_ok(req);
        resp.headers.insert({ "Content-Type", "text/html;charset=UTF-8" });
        resp.headers.insert({ "Connection", "keep-alive" });
        resp.content = "Hello";
        std::cout << "New get" << '\n';
        co_return resp;
      });
  server.set_router(std::move(router));
  server.run();
  return 0;
}
