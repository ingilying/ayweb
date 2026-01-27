#pragma once
#include <asio.hpp>
#include <tmc/all_headers.hpp>
#include <tmc/asio/aw_asio.hpp>
#include <tmc/asio/ex_asio.hpp>
#include <utility>

namespace ayweb
{
  class http_server
  {
   public:
    constexpr static unsigned int BUFFER_SIZE = 1024 * 2;

    http_server(asio::ip::address addr, unsigned short port)
        : m_address(std::move(addr)),
          m_acceptor(tmc::asio_executor(), { m_address, port }),
          m_port(port)
    {
    }

    int run()
    {
      tmc::asio_executor().init();
      tmc::cpu_executor().init();
      return tmc::async_main(accept_loop());
    }

    asio::ip::address get_address()
    {
      return m_address;
    }

    [[nodiscard]] unsigned short get_port() const
    {
      return m_port;
    }

   private:
    tmc::task<int> accept_loop()
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

    static tmc::task<void> socket_handler(asio::ip::tcp::socket socket)
    {
      char* data = new char[BUFFER_SIZE];  // NOLINT
      while (socket.is_open())
      {
        auto data_buf = asio::buffer(data, BUFFER_SIZE);
        auto [error, num] = co_await socket.async_read_some(data_buf, tmc::aw_asio);
        if (error)
        {
          socket.close();
          co_return;
        }
      }
    }

    asio::ip::address m_address;
    asio::ip::tcp::acceptor m_acceptor;
    unsigned short m_port;
  };
}  // namespace ayweb
