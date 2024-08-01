/*
 * 由豆包模型生成：
 * Prompt: 用c语言实现一个高性能http服务器，按如下要求:
 * 1）单进程支持最大数量的客户端高并发
 * 2）使用epoll实现高并发
 * 3）客户端 fd 注册为ET模式避免事件一直触发
 * 4）服务端 fd 注册为LT模式
 * 5）能识别并处理客户端正常关闭和异常关闭
 * 6) 读取完客户端的请求，解析，识别正确的HTTP GET请求再返回hello world响应
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <sys/epoll.h>
#include <errno.h>
#include "list.h"
#include "addr.h"

#define MAX_EVENTS 8192
#define BUFFER_SIZE 1024

static int setSockNonBlock(int so)
{
    //将socket设置为非阻塞的
    int oldSocketFlag = fcntl(so, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if (fcntl(so, F_SETFL, newSocketFlag) == -1)
    {
        perror("Fcntl failed");
        return -1;
    }

    return 0;
}

// 改进的 strstr 函数
static char *__strstr(const char *haystack, const char *needle) {
    int haystackLen = strlen(haystack);
    int needleLen = strlen(needle);

    if (needleLen == 0) {
        return (char *)haystack;
    }

    for (int i = 0; i <= haystackLen - needleLen; i++) {
        int j;
        for (j = 0; j < needleLen; j++) {
            if (haystack[i + j]!= needle[j]) {
                break;
            }
        }
        if (j == needleLen) {
            return (char *)&haystack[i];
        }
    }
    return NULL;
}

// 解析 HTTP 请求并处理
static void handleRequest(int client_fd, int epoll_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            // 客户端正常关闭
            printf("Client closed connection normally\n");
        } else {
            // 客户端异常关闭或读取错误
            perror("Error reading from client");
        }
        close(client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        return;
    }

    buffer[bytes_read] = '\0';

    // 检查是否为 HTTP GET 请求
    if (strncmp(buffer, "GET", 3) == 0) {
        // 简单的 HTTP 响应
        const char *response = "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!";
        send(client_fd, response, strlen(response), 0);
    }

    //close(client_fd);
    //epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    if (__strstr(buffer, "Connection: Close")) {
        close(client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage: %s [port]\n", argv[0]);
        return -1;
    }

    const char* addr = NULL;
    const char* port = strdup(argv[1]);
    Node* addrs = createList();
    localAddr(&addrs);

    for (Node* info = addrs; info != NULL; info = info->next) {
        if (strcmp(info->name, "lo") == 0) {
            addr = strdup(info->addr);
            printf("Listen to address %s:%s\n", addr, port);
            break;
        }
    }
    destroyList(&addrs);

    //创建一个监听socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("Socket failed");
        return -1;
    }

    //设置重用ip地址和端口号
    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (char*)& on, sizeof(on));
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, (char*)& on, sizeof(on));

    //将监听socker设置为非阻塞的
    if (setSockNonBlock(listen_fd) != 0) {
        return -1;
    }

    //初始化服务器地址
    struct sockaddr_in bindaddr;
    bindaddr.sin_family = AF_INET;
    bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    bindaddr.sin_port = htons(atoi(port));

    if (bind(listen_fd, (struct sockaddr*) & bindaddr, sizeof(bindaddr)) == -1)
    {
        perror("Bind failed");
        close(listen_fd);
        return -1;
    }

    //启动监听
    if (listen(listen_fd, SOMAXCONN) == -1)
    {
        perror("Listen failed");
        close(listen_fd);
        return -1;
    }

    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("Epoll creation failed");
        close(listen_fd);
        return -1;
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = listen_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) < 0) {
        perror("Epoll control failed for listen_fd");
        close(epoll_fd);
        close(listen_fd);
        return -1;
    }

    struct epoll_event events[MAX_EVENTS];

    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events < 0) {
            perror("Epoll wait failed");
            continue;
        }

        for (int i = 0; i < num_events; i++) {
            int fd = events[i].data.fd;

            if (fd == listen_fd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
                if (client_fd < 0) {
                    perror("Accept failed");
                    continue;
                }

                // 设置客户端套接字为非阻塞模式
                setSockNonBlock(client_fd);
                if (setsockopt(client_fd, SOL_TCP, TCP_NODELAY, &on, sizeof(on))) {
                    perror("set TCP NODELAY");
                    continue;
                }

                // 注册客户端读事件为 ET 模式
                // 使用 ET 模式下即使给客户端 fd 注册了检测可写事件不会一直触发，
                // 只会触发一次，触发完后只有再次注册检测可写事件才会继续触发。
                event.events = EPOLLIN | EPOLLOUT;
                // 取消注释这一行，则使用LT模式
                event.events |= EPOLLET;
                event.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
            } else if (events[i].events & EPOLLIN) {
                handleRequest(fd, epoll_fd);
            } else if (events[i].events & EPOLLOUT) {
                // 处理写事件
                // printf("Write event triggered for fd: %d\n", fd);
                // 可以在此处进行数据发送等操作
            }
        }
    }

    close(listen_fd);
    close(epoll_fd);

    return 0;
}
