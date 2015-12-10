#include "lib/network.hpp"
#include "lib/http.hpp"
#include "lib/promise-polyfill.hpp"
#include <cstdio>

cs477::net::http_response make_response(int status, const std::string &json)
{
	cs477::net::http_response rsp;
	rsp.status = status;
	switch (status)
	{
	case 200:{
		rsp.message = "Ok";
		break;
	}
	case 404:{
		rsp.message = "Not Found";
		break;
	}
	case 500:{
		rsp.message = "Internal Error";
		break;
	}
	}

	if (status == 200)
	{
		if (json.length())
		{
			rsp.body = json;
			rsp.headers.emplace_back("Content-Type", "application/json");
		}
	}

	return rsp;
}

void socket_handler(cs477::net::socket sock) {
  auto f = cs477::net::read_http_request_async(sock).share();
  Promise::then(f, [sock](auto s) {
	  auto rq = s.get();
	  int status = 200;
	  std::string result = "Working baby!";
	  auto rsp = make_response(200, result);
	  cs477::net::write_http_response_async(sock, rsp);
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
