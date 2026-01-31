#define TMC_IMPL
#include <gtest/gtest.h>

#include <cstdio>
#include <optional>
#include <tmc/all_headers.hpp>

#include "ayweb/protocol.hpp"
#include "ayweb/router.hpp"

TEST(PROTOCOL_TEST, READ)
{
  char msg[] = "GET /main.html?a=b&c=d&e=f HTTP/1.1\r\nHost: www.test.com\r\nUser-Agent: test te1a\r\n\r\n";  // NOLINT
  auto req = ayweb::read_message(msg, sizeof(msg));                                                           // NOLINT
  ASSERT_TRUE(req.has_value());
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  const auto& request = *req;
  ASSERT_EQ(request.method, "GET");
  ASSERT_EQ(request.path, "/main.html");
  ASSERT_EQ(request.params.at("a"), "b");
  ASSERT_EQ(request.params.at("c"), "d");
  ASSERT_EQ(request.params.at("e"), "f");
  ASSERT_EQ(request.version, "HTTP/1.1");
  ASSERT_EQ(request.headers.at("Host"), "www.test.com");
  ASSERT_EQ(request.headers.at("User-Agent"), "test te1a");
  ASSERT_EQ(request.content.empty(), true);
}

TEST(ROUTER_TEST, PATH_TREE)
{
  const std::string url = "/profile/jack/settings";
  ayweb::PathTree tree;
  tree.insert(
      url,
      { "GET",
        [](ayweb::Request req) -> tmc::task<ayweb::Response>
        {
          std::printf("success for settings");  // NOLINT
          co_return ayweb::Response{
            .version = "HTTP/1.1", .code = 200, .status = "OK", .headers = {}, .content = ""  // NOLINT
          };
        } });
  auto lazy = tree.search({ url, "GET" });
  ASSERT_TRUE(lazy.has_value());
}

TEST(MAIN_TEST, MAIN)
{
  tmc::async_main(
      []() -> tmc::task<int>
      {
        // ayweb::HttpServer server{ asio::ip::address_v4::any(), 8090 };  // NOLINT
        ayweb::Router router;
        router.route(
            "/",
            [](ayweb::Request req) -> tmc::task<ayweb::Response>
            {
              std::printf("handle /\n");  // NOLINT
              // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers,clang-analyzer-core.CallAndMessage)
              co_return ayweb::Response{ .version = "HTTP/1.1", .code = 200, .status = "OK", .headers = {}, .content = "" };
            });
        const ayweb::Request req{
          .method = "GET", .path = "/", .params = {}, .version = "HTTP/1.1", .headers = {}, .content = ""
        };
        co_await router.handle(req);
        co_return 0;
      }());
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
