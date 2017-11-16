#include "../include/common.h"

#include <linux/if_tun.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <linux/if.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>

namespace vpn {

static const int MAX_EVENTS = 512;

Tun::Tun(): _fd(-1), _ip(), _name() {
    init();
}

Tun::Tun(const std::string& addr) {
    init();
    std::string command;
    command = "ip addr add " + addr + " dev " + _name;
    assert(system(command.c_str()) == 0);
}

Tun::~Tun() {
    close(_fd);
}

int Tun::up() {
    std::string command;
    command = "ip link set dev " + _name + " up";
    return system(command.c_str());
}

void Tun::init() {
    _fd = open("/dev/net/tun", O_RDWR);
    assert(_fd >= 0);

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;

    assert(ioctl(_fd, TUNSETIFF, &ifr) == 0);

    _name = ifr.ifr_name;
}

Socket::Socket(Domain d, Type t) : _fd(-1), _type(-1), _domain(-1) {
    switch(d) {
        case IPv4:
            _domain = AF_INET;
            break;
        case IPv6:
            _domain = AF_INET6;
            break;
        default:
            /* nerver do this */
            assert(0);
    }
    switch(t) {
        case TCP:
            _type = SOCK_STREAM;
            break;
        case UDP:
            _type = SOCK_DGRAM;
            break;
        default:
            /* nerver do this */
            assert(0);
    }
    _fd = socket(_domain, _type, 0);
}

Socket::~Socket() {
    close(_fd);
}

int Socket::bind(int port) {
    assert(_type == SOCK_DGRAM);

    struct sockaddr_in sock;
    memset(&sock, 0, sizeof(sock));
    sock.sin_family = _domain;
    sock.sin_port = htons(static_cast<in_port_t>(port));
    sock.sin_addr.s_addr = htonl(INADDR_ANY);

    return ::bind(_fd, reinterpret_cast<struct sockaddr*>(&sock), sizeof(sock));
}

int Socket::sendto(const char* in, int size, const std::string& addr, int port) {
    assert(_type == SOCK_DGRAM);

    struct sockaddr_in sock;
    memset(&sock, 0, sizeof(sock));
    sock.sin_family = _domain;
    sock.sin_port = htons(static_cast<in_port_t>(port));
    assert(inet_pton(_domain, addr.c_str(), &sock.sin_addr) == 1);

    return ::sendto(_fd, in, size, 0,
            reinterpret_cast<struct sockaddr*>(&sock), sizeof(sock));
}

int Socket::recvfrom(char* out, int size, struct sockaddr* src, socklen_t* len) {
    assert(_type == SOCK_DGRAM);

    return ::recvfrom(_fd, out, size, 0, src, len);
}

Epoll::Epoll(): _fd(-1) {
    _fd = epoll_create(MAX_EVENTS);
}

int Epoll::add_read_event(int fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    return epoll_ctl(_fd, EPOLL_CTL_ADD, fd, &ev);
}

std::vector<struct epoll_event> Epoll::wait() {
    struct epoll_event events[MAX_EVENTS];
    int nwait = epoll_wait(_fd, events, MAX_EVENTS, -1);
    assert(nwait != -1);
    return std::vector<struct epoll_event>(events, events + nwait);
}

Epoll::~Epoll() {
    close(_fd);
}

} /* namespace vpn */
