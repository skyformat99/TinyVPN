#include <arpa/inet.h>

#include "server.h"

#include "gflags/gflags.h"

DEFINE_string(tun_addr, "", "tun's address. eg: 127.0.0.1");
DEFINE_int32(port, -1, "server's port. eg: 5003");

static bool validate_addr(const char* flagname, const std::string& value) {
    struct in_addr addr;
    return inet_pton(AF_INET, value.c_str(), &addr);
}

static bool validate_port(const char* flagname, int value) {
    return value >= 1 && value <= 65535;
}

DEFINE_validator(tun_addr, validate_addr);
DEFINE_validator(port, validate_port);

int main(int argc, char *argv[]) {
    google::SetUsageMessage("sudo server --tun_addr <addr> --port <port>");
    google::SetVersionString("1.0.0");
    google::ParseCommandLineFlags(&argc, &argv, true);

    vpn::Server server(FLAGS_tun_addr, FLAGS_port);
    server.run();
    return 0;
}
