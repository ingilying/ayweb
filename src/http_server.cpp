#include "ayweb/http_server.hpp"

#include <asio/buffers_iterator.hpp>
#include <asio/completion_condition.hpp>
#include <asio/read_until.hpp>
#include <asio/streambuf.hpp>
#include <memory>
#include <utility>

#include "ayweb/connection.hpp"
#include "ayweb/context.hpp"

namespace ayweb
{
  HttpServer::HttpServer(asio::ip::address addr, unsigned short port)
      : m_address(std::move(addr)),
        m_acceptor(tmc::asio_executor(), { m_address, port }),
        m_port(port)
  {
    this->gctx = std::make_shared<GlobalContext>();
  }

  void HttpServer::set_router(Router&& router)
  {
    this->gctx->router = std::move(router);
  }

  int HttpServer::run()
  {
    tmc::asio_executor().init();
    tmc::cpu_executor().init();
    return tmc::async_main(accept_loop());
  }

  asio::ip::address HttpServer::get_address()
  {
    return m_address;
  }

  unsigned short HttpServer::get_port() const
  {
    return m_port;
  }

  tmc::task<int> HttpServer::accept_loop()
  {
    auto forkg = tmc::fork_group();
    auto res = 0;
    while (true)
    {
      auto [error, socket] = co_await m_acceptor.async_accept(tmc::aw_asio);
      if (error)
      {
        res = -1;
        break;
      }
      forkg.fork(socket_handler(std::move(socket)));
    }
    co_await std::move(forkg);
    co_return res;
  }

  tmc::task<void> HttpServer::socket_handler(asio::ip::tcp::socket socket)
  {
    auto connection = Connection(std::move(socket));
    co_await connection.handle(gctx);
  }
}  // namespace ayweb
