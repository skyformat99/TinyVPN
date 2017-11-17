#ifndef VPN_DEVICE_H
#define VPN_DEVICE_H

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <string>
#include <vector>

namespace vpn {

class Tun {
public:
    Tun();
    Tun(const std::string& addr);
    ~Tun();
    Tun(const Tun&) = delete;
    Tun& operator=(const Tun&) = delete;

    int up();

    int fd() { return _fd; }
    std::string ip() { return _ip; }
    std::string name() { return _name; }

    int write(const void* in, int size) { return ::write(_fd, in, size); }
    int read(char* out, int size) { return ::read(_fd, out, size); }
private:
    int  _fd;
    std::string _ip;
    std::string _name;

    void init();
};

class Socket {
public:
    enum Domain {
        IPv4 = 0,
        IPv6,
    };
    enum Type {
        TCP = 0,
        UDP
    };
    Socket(Domain d, Type t);
    ~Socket();
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    int fd() { return _fd; }

    // just support UDP now
    int bind(int port);
    int sendto(const void* in, int size, const std::string& addr, int port);
    int recvfrom(char* out, int size, struct sockaddr* src, socklen_t* len);
private:
    int _fd;
    int _type;
    int _domain;
};

class Epoll {
public:
    Epoll();
    ~Epoll();
    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;

    int add_read_event(int fd);
    std::vector<struct epoll_event> wait();
private:
    int  _fd;
};

} /* namespace vpn */

#endif
