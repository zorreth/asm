#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

void serve(int sock_fd) {
    struct sockaddr_in client_addr;
    char buf[1024];
    socklen_t len;

    while (1) {
        memset(buf, 0, sizeof(buf));
        len = sizeof(client_addr);

        int conn_fd = accept(sock_fd, (struct sockaddr*)&client_addr, &len);
        if (conn_fd == -1) {
            perror("warning: server accept failed");
            continue;
        }

        ssize_t bytes_read = read(conn_fd, buf, sizeof(buf) - 1);

        if (bytes_read <= 0) {
            close(conn_fd);
            continue;
        }

        buf[bytes_read] = '\0';

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

        if (path == NULL) {
            char *bad_req = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            write(conn_fd, bad_req, strlen(bad_req));
            close(conn_fd);
            continue;
        }

        char full_path[256];

        size_t path_len = strlen(path);
        if (path_len > 0 && path[path_len - 1] == '/') {
            snprintf(full_path, sizeof(full_path), "html%sindex.html", path);
        } else {
            snprintf(full_path, sizeof(full_path), "html%s", path);
        }

        FILE *fp = NULL;
        char *headers;
        struct stat file_stat;

        if (stat(full_path, &file_stat) != 0 || S_ISDIR(file_stat.st_mode)) {
            fp = fopen("html/404.html", "r");
            headers = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
        } else {
            fp = fopen(full_path, "r");

            if (fp == NULL) {
                fp = fopen("html/403.html", "r");
                headers = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\n\r\n";
            } else {
                headers = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            }            
        }

        write(conn_fd, headers, strlen(headers));

        char file_buf[1024];

        while ((bytes_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
            write(conn_fd, file_buf, bytes_read);
        }

        fclose(fp);
        close(conn_fd);
    }
}

int main() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("socket creation failed...");
        return 1;
    }

    struct sockaddr_in server_addr;
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

    serve(sock_fd);

    close(sock_fd);
    return 0;
}
