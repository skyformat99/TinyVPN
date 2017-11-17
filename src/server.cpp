#include "server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <assert.h>

#ifdef DEBUG
#include <iostream>
#endif

namespace vpn {

static const int MAX_EVENTS = 512;
static const int MAX_PORT = 65536;

Server::Server(const std::string& addr, int port)
    : _socket(Socket::IPv4, Socket::UDP), _epoll(), _tun(addr), _port(port),
    _snat_map(), _port_pool(MAX_PORT, false) {
    // for Tins library
    assert(sizeof(char) == sizeof(uint8_t));

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
                int nread = _tun.read(buf, sizeof(buf));
                assert(nread != -1);
#ifdef DEBUG
                std::cout << "recvfrom tun: " << _tun.ip() << std::endl;
#endif

                std::shared_ptr<Tins::IP> ip = get_ip_packet(reinterpret_cast<uint8_t*>(buf), nread);
                if (ip == nullptr) {
                    continue;
                }

                int port = get_dport(ip);
                if (port == -1
                        || _snat_map.find({_tun.ip(), port}) == _snat_map.end()) {
                    continue;
                }

                /* pair<addr. port> */
                AddrPort origin = _snat_map[{_tun.ip(), port}];
                ip->dst_addr(origin.first);
                assert(_sock_map.find(origin) != _sock_map.end());
                AddrPort sock_origin = _sock_map[origin];

#ifdef DEBUG
                std::cout << "send to socket: " << sock_origin.first.to_string() << ":" << htons(sock_origin.second) << std::endl;
#endif

                Tins::IP::serialization_type data = ip->serialize();
                assert(_socket.sendto(data.data(), nread, sock_origin.first.to_string(), sock_origin.second) == nread);
            } else if (event.data.fd == _socket.fd()) {
                struct sockaddr_in sock;
                socklen_t len = sizeof(sock);
                int nread = _socket.recvfrom(buf, sizeof(buf),
                        reinterpret_cast<struct sockaddr*>(&sock), &len);
                assert(nread != -1);
                char sock_addr[32];
                inet_ntop(AF_INET, &sock.sin_addr, sock_addr, sizeof(sock_addr));

#ifdef DEBUG
                std::cout << "recvfrom socket: " << sock_addr << ":" << sock.sin_port << std::endl;
#endif

                std::shared_ptr<Tins::IP> ip = get_ip_packet(reinterpret_cast<uint8_t*>(buf), nread);
                if (ip == nullptr) {
                    continue;
                }

                Tins::IP::address_type src_addr = ip->src_addr();
                int port = get_sport(ip);
                if (port == -1) {
                    continue;
                }

                _snat_map[{_tun.ip(), port}] = {src_addr, port};
                _sock_map[{src_addr, port}] = {sock_addr, ntohs(sock.sin_port)};
                ip->src_addr(_tun.ip());

                Tins::IP::serialization_type data = ip->serialize();
                _tun.write(data.data(), data.size());

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

std::shared_ptr<Tins::IP> Server::get_ip_packet(const uint8_t* buf, int size) {
    try {
        return std::make_shared<Tins::IP>(buf, size);
    } catch (...) {
        return nullptr;
    }
}

int Server::get_sport(std::shared_ptr<Tins::IP> ip) {
    if (ip->protocol() == IPPROTO_TCP) {
        Tins::TCP *tcp = ip->find_pdu<Tins::TCP>();
        return tcp->sport();
    } else if (ip->protocol() == IPPROTO_UDP) {
        Tins::UDP *udp = ip->find_pdu<Tins::UDP>();
        return udp->sport();
    } else {
        return -1;
    }
}

int Server::get_dport(std::shared_ptr<Tins::IP> ip) {
    if (ip->protocol() == IPPROTO_TCP) {
        Tins::TCP *tcp = ip->find_pdu<Tins::TCP>();
        return tcp->dport();
    } else if (ip->protocol() == IPPROTO_UDP) {
        Tins::UDP *udp = ip->find_pdu<Tins::UDP>();
        return udp->dport();
    } else {
        return -1;
    }
}

} /* namespace vpn */
