#pragma once
#include <asio.hpp>
#include <asio/error_code.hpp>
#include <asio/ip/tcp.hpp>
#include <tmc/all_headers.hpp>
#include <tmc/asio/aw_asio.hpp>
#include <tmc/asio/ex_asio.hpp>

#include "ayweb/router.hpp"
#include "tmc/task.hpp"

namespace ayweb
{
  class HttpServer
  {
   public:
    constexpr static unsigned int BUFFER_SIZE = 1024 * 2;

    HttpServer() = delete;
    HttpServer(asio::ip::address addr, unsigned short port);

    void set_router(Router&& router);
    int run();
    asio::ip::address get_address();
    [[nodiscard]] unsigned short get_port() const;

   private:
    tmc::task<int> accept_loop();
    tmc::task<void> socket_handler(asio::ip::tcp::socket socket);
    static std::string read_line(asio::streambuf &sbuf, asio::ip::tcp::socket &socket);
    asio::ip::address m_address;
    asio::ip::tcp::acceptor m_acceptor;
    unsigned short m_port;
    Router m_router;
  };

  
}  // namespace ayweb
