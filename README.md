# ahttpd
通过httpd不同实现方式，研究各种高性能io和网络模型的最佳性能

目前包括2个实现:
* ahttpd: 默认基于epoll实现
* ahttpd.ev: 基于libevent的实现

# 编译

1. On ubuntu, 先安装libevent

```bash
apt install libeventlibevent-dev
```
2. 编译

```bash
# ahttpd
make
# ahttpd.ev
make ev
```

# 测试

1. 启动服务端监听本地指定端口:

ahttpd:

```
./ahttpd 2222
```

ev_httpd:
```
./ahttpd.ev 2222
```

2. 客户端依赖使用 [wrk](https://github.com/wg/wrk), 默认4 threads，256 connects

```
wrk -t 4 -c 256 -d 10s http://localhost:2222
```

# 测试结果
## 长连接
1. ahttpd
```
./wrk -t 4 -c 256 -d 10s http://localhost:2222
Running 10s test @ http://localhost:2222
  4 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     1.81ms  717.76us   5.02ms   71.42%
    Req/Sec    35.27k     3.50k   52.97k    68.75%
  1403436 requests in 10.03s, 68.26MB read
Requests/sec: 139860.68
Transfer/sec:      6.80MB
```

2. ahttpd.ev
```
./wrk -t 4 -c 256 -d 10s http://localhost:2222
Running 10s test @ http://localhost:2222
  4 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     4.18ms    0.91ms   8.35ms   77.29%
    Req/Sec    15.38k   732.91    27.48k    93.50%
  612162 requests in 10.03s, 100.41MB read
Requests/sec:  61029.34
Transfer/sec:     10.01MB
```

3. nginx (默认index页面，worker_processes 1)

```
./wrk -t 4 -c 256 -d 10s http://localhost
Running 10s test @ http://localhost
  4 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     8.41ms   12.05ms 192.85ms   95.37%
    Req/Sec     9.92k     1.02k   19.08k    78.00%
  394470 requests in 10.04s, 323.13MB read
Requests/sec:  39296.07
Transfer/sec:     32.19MB
```
## 短连接
1. ahttpd
```
./wrk -t 4 -c 256 -d 10s http://localhost:2222 -H 'Connection: Close'
Running 10s test @ http://localhost:2222
  4 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     3.72ms    1.84ms   7.94ms   66.56%
    Req/Sec    14.99k     1.22k   20.16k    83.50%
  596655 requests in 10.05s, 29.02MB read
  Socket errors: connect 0, read 596626, write 0, timeout 0
Requests/sec:  59350.01
Transfer/sec:      2.89MB
```

2. ahttpd.ev

```
./wrk -t 4 -c 256 -d 10s http://localhost:2222 -H 'Connection: Close'
Running 10s test @ http://localhost:2222
  4 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     4.32ms    6.17ms 214.12ms   99.76%
    Req/Sec     9.51k     1.93k   15.35k    67.50%
  378620 requests in 10.03s, 68.97MB read
  Socket errors: connect 0, read 4, write 0, timeout 0
Requests/sec:  37739.64
Transfer/sec:      6.87MB
```

3. nginx (默认index页面，worker_processes 1)

```
./wrk -t 4 -c 256 -d 10s http://localhost -H 'Connection: Close'
Running 10s test @ http://localhost
  4 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     7.57ms    2.51ms  15.21ms   70.46%
    Req/Sec     8.14k   424.58    13.40k    82.50%
  323975 requests in 10.04s, 263.86MB read
Requests/sec:  32277.47
Transfer/sec:     26.29MB
```

