#include "../include/client.h"

#include <assert.h>

namespace vpn {

Client::Client(const std::string& addr, int port)
    : _socket(Socket::IPv4, Socket::UDP), _epoll(), _tun("192.168.10.1"),
    _srv_port(port), _srv_addr(addr) {
    assert(_epoll.add_read_event(_tun.fd()) == 0);
    assert(_epoll.add_read_event(_socket.fd()) == 0);
}

void Client::run() {
    assert(_tun.up() == 0);
    char buf[4096];
    for ( ; ; ) {
        std::vector<struct epoll_event> events(_epoll.wait());

        for (const auto& event : events) {
            if (event.data.fd == _tun.fd()) {
                int nread = _tun.read(buf, sizeof(buf));
                assert(nread != -1);
                assert(_socket.sendto(buf, nread, _srv_addr, _srv_port) != -1);
            } else if (event.data.fd == _socket.fd()) {
                int nread = _socket.recvfrom(buf, sizeof(buf), nullptr, nullptr);
                assert(nread != -1);
                assert(_tun.write(buf, nread) != -1);
            } else {
                /* nerver do this */
                assert(0);
            }
        }
    }
}

} /* namespace vpn */
