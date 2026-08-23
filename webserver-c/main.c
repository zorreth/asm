#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

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

    int opt = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) {
        perror("socket bind failed");
        return 1;
    }

    if (listen(sock_fd, 5) != 0) {
        perror("socket listen failed");
        return 1;
    }

    printf("Server is listening on port 8080...\n");

    char buf[1024];
    socklen_t len;

    while (1) {
        memset(buf, 0, sizeof(buf));
        len = sizeof(client_addr);

        conn_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &len);
        if (conn_fd == -1) {
            perror("server accept failed");
            return 1;
        }

        read(conn_fd, buf, sizeof(buf));

        char *path = NULL;

        for (int i = 0; buf[i] != '\0'; i++) {
            if (buf[i] == ' ') {
                if (path == NULL) {
                    path = &buf[i + 1];
                } else {
                    buf[i] = '\0';
                    break;
                }
            }
        }

        char *reply;

        if (strcmp(path, "/") == 0) {
            reply = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nHome Page";
        } else if (strcmp(path, "/about") == 0) {
            reply = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nAbout Page";
        } else {
            reply = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nNot Found";
        }

        write(conn_fd, reply, strlen(reply));
        close(conn_fd);
    }

    close(sock_fd);
    return 0;
}
