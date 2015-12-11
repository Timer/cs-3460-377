#include "../lib/network.hpp"
#include "../lib/http.hpp"
#include "../lib/promise-polyfill.hpp"
#include <cstdio>
#include "../lib/file.hpp"

#define PATH_DISABLED false

cs477::net::http_response make_response(int status, const std::string &content, const std::string &contentType) {
  cs477::net::http_response rsp;
  rsp.status = status;
  switch (status) {
  case 200: {
    rsp.message = "Ok";
    break;
  }
  case 404: {
    rsp.message = "Not Found";
    break;
  }
  case 500: {
    rsp.message = "Internal Error";
    break;
  }
  }

  if (content.length()) {
    rsp.body = content;
    rsp.headers.emplace_back("Content-Type", contentType);
  }

  return rsp;
}

void socket_handler(cs477::net::socket sock) {
  auto f = cs477::net::read_http_request_async(sock).share();
  Promise::then(f, [sock](auto s) {
    auto rq = s.get();
    std::string path = rq.url;
    auto pos = path.find("/");
    if (pos != std::string::npos) {
#if PATH_DISABLED
      for (;;) {
        if (pos + 1 >= path.length()) break;
        auto pos2 = path.find("/", pos + 1);
        if (pos2 != std::string::npos)
          pos = pos2;
        else
          break;
      }
#endif
      path = path.substr(pos + 1);
      if (path.length() < 1) {
        cs477::net::write_http_response_async(sock, make_response(404, "File not found.", "text/plain"));
      } else {
        try {
          Promise::then(read_file_async(path.c_str()).share(), [sock](auto f) {
            auto s = f.get();
            cs477::net::write_http_response_async(sock, make_response(200, s, "text/plain"));
          });
        } catch (...) {
          cs477::net::write_http_response_async(sock, make_response(404, "File not found.", "text/plain"));
        }
      }
    } else {
      cs477::net::write_http_response_async(sock, make_response(404, "File not found.", "text/plain"));
    }
  });
}

int main(int argc, char **argv) {
  cs477::net::initialize();
  auto addr = cs477::net::resolve_address("localhost", 8080);
  auto host = std::make_shared<cs477::net::acceptor>();
  host->listen(addr);

  for (int i = 0; i < 32; i++) {
    host->accept_async(socket_handler);
  }

  std::promise<void> p;
  auto f = p.get_future();
  f.wait();
  cs477::net::finalize();
  return 0;
}
