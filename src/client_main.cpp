#include <arpa/inet.h>

#include "client.h"

#include "gflags/gflags.h"

DEFINE_string(srv_addr, "", "server's address. eg: 127.0.0.1");
DEFINE_int32(srv_port, -1, "server's port. eg: 5003");

static bool validate_addr(const char* flagname, const std::string& value) {
    struct in_addr addr;
    return inet_pton(AF_INET, value.c_str(), &addr);
}

static bool validate_port(const char* flagname, int value) {
    return value >= 1 && value <= 65535;
}

DEFINE_validator(srv_addr, validate_addr);
DEFINE_validator(srv_port, validate_port);

int main(int argc, char *argv[]) {
    google::SetUsageMessage("sudo client --srv_addr <addr> --srv_port <port>");
    google::SetVersionString("1.0.0");
    google::ParseCommandLineFlags(&argc, &argv, true);

    vpn::Client client(FLAGS_srv_addr, FLAGS_srv_port);
    client.run();
    return 0;
}
