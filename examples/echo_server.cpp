#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int port = 9090;
    if(argc > 1) port = atoi(argv[1]);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 5);
    std::cout << "Echo server listening on port " << port << std::endl;
    int client = accept(sock, nullptr, nullptr);
    char buf[1024];
    ssize_t n = read(client, buf, sizeof(buf));
    if(n > 0) {
        write(client, buf, n);
    }
    close(client);
    close(sock);
    return 0;
}
