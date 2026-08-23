#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    int sock_fd, conn_fd;
    struct sockaddr_in server_addr, client_addr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket creation failed...");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("socket bind failed");
        return 1;
    }

    if (listen(sock_fd, 5) != 0) {
        perror("socket listen failed");
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    socklen_t len = sizeof(client_addr);

    conn_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &len);
    if (conn_fd == -1) {
        perror("server accept failed");
        return 1;
    }

    close(sock_fd);
    return 0;
}
