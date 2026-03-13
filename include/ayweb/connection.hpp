#pragma once

#include <asio/ip/tcp.hpp>
#include <memory>

#include "ayweb/router.hpp"
#include "tmc/task.hpp"

namespace ayweb
{
  class Connection
  {
    using AsioSocket = asio::ip::tcp::socket;

   public:
    explicit Connection(AsioSocket&& sock,Router route);
    tmc::task<void> handle();
    tmc::task<void> send(std::string data);
    tmc::task<void> send_chunk(std::string data);
    tmc::task<void> end_chunks();

   private:
    std::unique_ptr<AsioSocket> socket;
    Router router;
  };
};  // namespace ayweb