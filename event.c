/*
 * 由豆包模型生成，
 * Prompt: 用C语言写一个基于libevent写一个单进程的http服务器，返回hello world
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>

void http_request_cb(struct evhttp_request *req, void *arg) {
    struct evbuffer *buf = evbuffer_new();
    if (!buf) {
        perror("Failed to create buffer");
        return;
    }

    evbuffer_add_printf(buf, "HTTP/1.1 200 OK\r\nContent-Length: 12\r\n\r\nHello World!");
    evhttp_send_reply(req, HTTP_OK, "OK", buf);
    evbuffer_free(buf);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s [port]\n", argv[0]);
        return -1;
    }
    int port = atoi(argv[1]);

    struct event_base *base;
    struct evhttp *http;

    base = event_base_new();
    if (!base) {
        perror("Failed to create event base");
        return 1;
    }

    http = evhttp_new(base);
    if (!http) {
        perror("Failed to create HTTP");
        event_base_free(base);
        return 1;
    }

    if (evhttp_bind_socket(http, "0.0.0.0", port) < 0) {
        perror("Failed to bind socket");
        evhttp_free(http);
        event_base_free(base);
        return 1;
    }

    evhttp_set_gencb(http, http_request_cb, NULL);

    event_base_dispatch(base);

    evhttp_free(http);
    event_base_free(base);

    return 0;
}
