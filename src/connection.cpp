#include "ayweb/connection.hpp"

#include <asio/read.hpp>
#include <asio/read_until.hpp>
#include <asio/streambuf.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <tmc/asio/aw_asio.hpp>
#include <tmc/task.hpp>
#include <utility>

#include "ayweb/context.hpp"
#include "ayweb/protocol.hpp"
#include "ayweb/router.hpp"

namespace ayweb
{
  Connection::Connection(AsioSocket&& sock)
  {
    socket = std::make_unique<AsioSocket>(std::move(sock));
  }

  tmc::task<void> Connection::send(std::string data)
  {
    auto [ecode, size] = co_await socket->async_send(asio::buffer(data), tmc::aw_asio);
    if (ecode || size != data.size())
    {
      std::cout << "Error when sending data: " << ecode.message() << '\n';
    }
  }

  tmc::task<void> Connection::send_head(Response resp)
  {
    auto data = output_response(resp);
    co_await this->send(std::move(data));
  }

  tmc::task<void> Connection::send_chunk(std::string data)
  {
    std::string chunk;
    chunk += std::to_string(data.size()) + "\r\n";
    chunk += data + "\r\n";
    co_await send(std::move(chunk));
  }

  tmc::task<void> Connection::end_chunks()
  {
    constexpr static std::string chunk_end = "0\r\n\r\n";
    co_await send(chunk_end);
  }

  static tmc::task<void>
  handle_content_length(asio::ip::tcp::socket& socket, asio::streambuf& streambuf, Request& req)  // NOLINT
  {
    auto len_itr = req.headers.find("Content-Length");
    if (len_itr != req.headers.end())
    {
      auto [_, len_str] = *len_itr;  // NOLINT
      int content_len = 0;
      auto res = std::from_chars(len_str.begin(), len_str.end(), content_len);  // NOLINT
      if (res.ec != std::errc{} || content_len != 0)
      {
        auto [ecode, bytes_transfer] =
            co_await asio::async_read(socket, streambuf, asio::transfer_exactly(content_len), tmc::aw_asio);
        if (ecode)
        {
          socket.close();
          co_return;
        }
        auto content_data = streambuf.data();
        std::string content{ asio::buffers_begin(content_data), asio::buffers_begin(content_data) + (long)bytes_transfer };
        streambuf.consume(bytes_transfer);
        req.content = std::move(content);
      }
    }
  }

  static tmc::task<void> handle_chunked(asio::ip::tcp::socket& socket, asio::streambuf& streambuf, Request& req)  // NOLINT
  {
    auto enc_itr = req.headers.find("Transfer-Encoding");
    if (enc_itr != req.headers.end())
    {
      auto enc = enc_itr->second;
      if (enc == "chunked")
      {
        auto [ecode, bytes_transfer] = co_await asio::async_read_until(socket, streambuf, "\r\n\r\n", tmc::aw_asio);
        if (ecode)
        {
          socket.close();
          co_return;
        }
        auto chunks_data = streambuf.data();
        std::string chunks_str{ asio::buffers_begin(chunks_data), asio::buffers_begin(chunks_data) + (long)bytes_transfer };
        streambuf.consume(bytes_transfer);
        auto content = read_chunks(chunks_str);
        if (content)
        {
          req.content = std::move(*content);
        }
      }
    }
  }

  tmc::task<void> Connection::handle(std::shared_ptr<GlobalContext> ctx)
  {
    asio::streambuf streambuf;
    while (socket->is_open())
    {
      auto [ecode, bytes_transfer] = co_await asio::async_read_until(*socket, streambuf, "\r\n\r\n", tmc::aw_asio);
      if (ecode)
      {
        socket->close();
        co_return;
      }
      auto header_data = streambuf.data();
      std::string header_str{ asio::buffers_begin(header_data), asio::buffers_begin(header_data) + (long)bytes_transfer };
      streambuf.consume(bytes_transfer);
      // parse header
      auto req = read_request(header_str);
      if (req.has_value())
      {
        // read content
        co_await handle_content_length(*socket, streambuf, *req);
        co_await handle_chunked(*socket, streambuf, *req);
        req->conn = this;
        // handle request
        auto response = co_await ctx->router.handle(std::move(*req));
        if (response)
        {
          if (response->option == RespOutputOption::Chunked)
          {
            continue;
          }
          auto response_data = output_response(*response);
          co_await socket->async_send(asio::buffer(response_data), tmc::aw_asio);
        }
      }
    }
    co_return;
  }
}  // namespace ayweb
