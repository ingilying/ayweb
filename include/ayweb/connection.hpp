#pragma once

#include <asio/ip/tcp.hpp>
#include <memory>

#include "ayweb/context.hpp"
#include "export.hpp"
#include "tmc/task.hpp"

namespace ayweb
{
  AYWEB_EXPORT class Connection
  {
    using AsioSocket = asio::ip::tcp::socket;

   public:
    explicit Connection(AsioSocket&& sock);
    tmc::task<void> handle(std::shared_ptr<GlobalContext> ctx);
    tmc::task<void> send(std::string data);
    tmc::task<void> send_head(Response resp);
    tmc::task<void> send_chunk(std::string data);
    tmc::task<void> end_chunks();

   private:
    std::unique_ptr<AsioSocket> socket;
  };
};  // namespace ayweb
