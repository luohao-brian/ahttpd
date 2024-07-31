### ahttpd

旨在通过httpd实现，对比各种高性能io和网络模型
一个基于epoll的高性能httpd。

* `epoll_httpd.c` - 基于原生epoll的httpd;
* `ev_httpd.c` - 基于libevent的httpd;

### 测试

服务端:

epoll_httpd:

```
gcc -o epoll_httpd epoll_httpd.c
./epoll_httpd 7777
```

ev_httpd:
```
gcc -o ev_httpd ev_httpd.c -levent
./ev_httpd 7777
```

客户端:

```
wrk -t 1 -c 256 -d 10s http://localhost:7777
```
