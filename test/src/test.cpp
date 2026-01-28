#include <gtest/gtest.h>
#include <optional>
#include "ayweb/protocol.hpp"

TEST(PROTOCOL_TEST, READ)
{
  char msg[] = "GET /main.html HTTP/1.1\r\nHost: www.test.com\r\nUser-Agent: test te1a\r\n\r\n";  // NOLINT
  auto req = ayweb::read_message(msg, sizeof(msg));                                               // NOLINT
  ASSERT_TRUE(req.has_value());
  // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
  const auto& request = *req;
  ASSERT_EQ(request.method, "GET");
  ASSERT_EQ(request.path, "/main.html");
  ASSERT_EQ(request.version, "HTTP/1.1");
  ASSERT_EQ(request.headers.at("Host"), "www.test.com");
  ASSERT_EQ(request.headers.at("User-Agent"), "test te1a");
  ASSERT_EQ(request.content.empty(), true);
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
