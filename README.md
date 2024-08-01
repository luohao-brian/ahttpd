# ahttpd
通过httpd不同实现方式，研究各种高性能io和网络模型的最佳性能

目前包括2个实现:
* ahttpd: 默认基于epoll实现
* ahttpd_ev: 基于libevent的实现

# 编译

1. On ubuntu, 先安装libevent

```bash
apt install libeventlibevent-dev
```
2. 编译

```bash
# ahttpd
make
# ahttpd_ev
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
./ahttpd_ev 2222
```

2. 客户端依赖使用 [wrk](https://github.com/wg/wrk)

```
wrk -t 1 -c 256 -d 10s http://localhost:2222
```

# 结果
1. ahttpd
```
# ./wrk -t 1 -c 256 -d 10s http://localhost:2222
Running 10s test @ http://localhost:2222
  1 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     2.75ms    1.38ms   7.95ms   67.02%
    Req/Sec    30.55k     1.86k   34.93k    67.00%
  305770 requests in 10.09s, 14.87MB read
  Socket errors: connect 0, read 305706, write 0, timeout 0
Requests/sec:  30314.75
Transfer/sec:      1.47MB
```

2. ahttpd_ev
```
# ./wrk -t 1 -c 256 -d 10s http://localhost:2222
Running 10s test @ http://localhost:2222
  1 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     3.84ms    1.27ms   8.10ms   66.03%
    Req/Sec    63.36k     1.16k   67.24k    67.00%
  630401 requests in 10.02s, 103.41MB read
Requests/sec:  62898.25
Transfer/sec:     10.32MB
```

3. nginx (默认index页面，worker_processes 1)

```
# ./wrk -t 1 -c 256 -d 10s http://localhost
Running 10s test @ http://localhost
  1 threads and 256 connections
  Thread Stats   Avg      Stdev     Max   +/- Stdev
    Latency     6.73ms    6.74ms 164.45ms   97.60%
    Req/Sec    40.93k     2.19k   46.97k    66.00%
  407003 requests in 10.03s, 333.40MB read
Requests/sec:  40577.42
Transfer/sec:     33.24MB
```
