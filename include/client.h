#ifndef VPN_CLIENT_H
#define VPN_CLIENT_H

#include <sys/socket.h>
#include <netinet/in.h>

#include <string>

#include "common.h"

namespace vpn {

class Client {
public:
    Client(const std::string& addr, int port);
    Client(const Client&) = delete;
    bool operator=(const Client&) = delete;

    void run();
private:
    Socket _socket;
    Epoll  _epoll;
    Tun    _tun;
    int    _srv_port;
    std::string  _srv_addr;
};

} /* namespace vpn */

#endif
