#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char** argv) {
    std::string server = "127.0.0.1";
    int port = 9090;
    if(argc > 1) server = argv[1];
    if(argc > 2) port = atoi(argv[2]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, server.c_str(), &addr.sin_addr);
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    std::string msg = "hello";
    write(sock, msg.c_str(), msg.size());
    char buf[1024];
    ssize_t n = read(sock, buf, sizeof(buf));
    if(n > 0) std::cout << "recv: " << std::string(buf, buf + n) << std::endl;
    close(sock);
    return 0;
}
