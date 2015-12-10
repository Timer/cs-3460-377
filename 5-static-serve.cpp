#include "lib/network.hpp"
#include <cstdio>

void socket_handler(cs477::net::socket sock) {
  puts("Need to handle a connection!");
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
