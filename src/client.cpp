#include "client.h"

#include <arpa/inet.h>
#include <assert.h>

#ifdef DEBUG
#include <iostream>
#endif

namespace vpn {

Client::Client(const std::string& addr, int port)
    : _socket(Socket::IPv4, Socket::UDP), _epoll(), _tun(),
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

#ifdef DEBUG
                std::cout << "recvfrom tun: " << _tun.ip() << std::endl;
#endif

                assert(nread != -1);
                assert(_socket.sendto(buf, nread, _srv_addr, _srv_port) == nread);

#ifdef DEBUG
                std::cout << "send to socket: " << _srv_addr << ":" << _srv_port << std::endl;
#endif

            } else if (event.data.fd == _socket.fd()) {
#ifdef DEBUG
                struct sockaddr_in src;
                socklen_t len;
                int nread = _socket.recvfrom(buf, sizeof(buf), reinterpret_cast<struct sockaddr*>(&src), &len);
                char ip[128];
                inet_ntop(AF_INET, &src.sin_addr, ip, sizeof(ip));
                std::cout << "recvfrom socket: " << ip << ":" << src.sin_port << std::endl;
#else
                int nread = _socket.recvfrom(buf, sizeof(buf), nullptr, nullptr);
#endif

                assert(nread != -1);
                assert(_tun.write(buf, nread) == nread);

#ifdef DEBUG
                std::cout << "write to tun: " << _tun.ip() << std::endl;
#endif
            } else {
                /* nerver do this */
                assert(0);
            }
        }
    }
}

} /* namespace vpn */
