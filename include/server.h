#ifndef VPN_SERVER_H
#define VPN_SERVER_H

#include <string>

#include "../include/common.h"

namespace vpn {

class Server {
public:
    Server(const std::string& addr, int port);
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void run();
private:
    Socket _socket;
    Epoll  _epoll;
    Tun    _tun;
    int    _port;
};


} /* namespace vpn */

#endif
