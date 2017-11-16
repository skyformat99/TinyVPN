#include "../include/server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <assert.h>

#include <iostream>

#include "../include/tins/tins.h"

namespace vpn {

static const int MAX_EVENTS = 512;

Server::Server(const std::string& addr, int port)
    : _socket(Socket::IPv4, Socket::UDP), _epoll(), _tun(addr), _port(port) {
    _epoll.add_read_event(_socket.fd());
    _epoll.add_read_event(_tun.fd());
}

void Server::run() {
    assert(_tun.up() == 0);
    assert(_socket.bind(_port) == 0);

    char buf[4096];
    for ( ; ; ) {
        std::vector<struct epoll_event> events(_epoll.wait());

        for (const auto& event : events) {
            if (event.data.fd == _tun.fd()) {
                continue;
            } else if (event.data.fd == _socket.fd()) {
                int nread = _socket.recvfrom(buf, sizeof(buf), nullptr, nullptr);
                assert(nread != -1);
                std::cout << nread << std::endl;

                assert(sizeof(char) == sizeof(uint8_t));
                try {
                    Tins::IP ip(reinterpret_cast<uint8_t*>(buf), nread);
                    Tins::IP::address_type src = ip.src_addr();
                    Tins::IP::address_type dst = ip.dst_addr();
                    std::cout << src << std::endl;
                    std::cout << dst << std::endl;
                } catch (...) {
                    std::cout << "error" << std::endl;
                }

            } else {
                /* nerver do this */
                assert(0);
            }
        }
    }
}

} /* namespace vpn */
