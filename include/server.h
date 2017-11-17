#ifndef VPN_SERVER_H
#define VPN_SERVER_H

#include <string>
#include <memory>
#include <map>

#include "common.h"

#include "tins/tins.h"

namespace vpn {

using AddrPort = std::pair<Tins::IP::address_type, int>;

class Server {
public:
    Server(const std::string& addr, int port);
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void run();
private:
    Socket  _socket;
    Epoll   _epoll;
    Tun     _tun;
    int     _port;

    std::map<AddrPort, AddrPort>  _snat_map;
    std::map<AddrPort, AddrPort>  _sock_map;
    std::vector<bool> _port_pool;

    std::shared_ptr<Tins::IP> get_ip_packet(const uint8_t* buf, int size);
    int get_sport(std::shared_ptr<Tins::IP> ip);
    int get_dport(std::shared_ptr<Tins::IP> ip);
};

} /* namespace vpn */

#endif
