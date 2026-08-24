#include <stdio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

typedef enum {
    TYPE_NOT_FOUND = -1,
    TYPE_FILE = 1,
    TYPE_DIR = 2
} PathType;

PathType pathtype(char *path) {
    struct stat path_stat;

    if (stat(path, &path_stat) == 0) {
        if (S_ISREG(path_stat.st_mode)) {
            return TYPE_FILE;
        } else if (S_ISDIR(path_stat.st_mode)) {
            return TYPE_DIR;
        }
    }

    return TYPE_NOT_FOUND;
}

void log_request(char *path, struct sockaddr_in client_addr) {
    time_t current_time;
    time(&current_time);

    char fmt_time[48];
    struct tm *tmp = localtime(&current_time);

    strftime(fmt_time, sizeof(fmt_time), "%Y-%m-%d %H:%M:%S", tmp);

    char addr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, addr, INET_ADDRSTRLEN);

    printf("%s %s %s\n", fmt_time, path, addr);
}

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
        snprintf(full_path, sizeof(full_path), "html%s", path);

        PathType type = pathtype(full_path);

        if (type == TYPE_DIR) {
            size_t current_len = strlen(full_path);
            size_t remaining_space = sizeof(full_path) - strlen(full_path) - 1;

            if (full_path[current_len - 1] == '/') {
                strncat(full_path, "index.html", remaining_space);
            } else {
                strncat(full_path, "/index.html", remaining_space);
            }

            type = pathtype(full_path);
        }

        FILE *fp = NULL;
        char *headers;
        
        if (type == TYPE_NOT_FOUND) {
            fp = fopen("html/404.html", "r");
            headers = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
        } else if (type == TYPE_FILE) {
            fp = fopen(full_path, "r");
            if (fp == NULL) {
                fp = fopen("html/403.html", "r");
                headers = "HTTP/1.1 403 Forbidden\r\nContent-Type: text/html\r\n\r\n";
            } else {
                headers = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
            }  
        }

        log_request(full_path, client_addr);

        write(conn_fd, headers, strlen(headers));

        char file_buf[1024];

        if (fp != NULL) {
            while ((bytes_read = fread(file_buf, 1, sizeof(file_buf), fp)) > 0) {
                write(conn_fd, file_buf, bytes_read);
            }
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
