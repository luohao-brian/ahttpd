//https://banu.com/blog/2/how-to-use-epoll-a-complete-example-in-c/epoll-example.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <errno.h>

// for setrlimit
#include <sys/time.h>
#include <sys/resource.h>


#define MAX_EVENTS 1<<15
#define SRV_BACKLOG 1<<10
#define MAX_OPEN_FILES 100000

static int ulimit_set_max_open_files(int max_files) {
    struct rlimit rl;

    rl.rlim_cur = max_files;
    rl.rlim_max = max_files;

    if (setrlimit(RLIMIT_NOFILE, &rl) < 0) {
        perror("setrlimit");
        return -1;
    }

    return 0;
}

static int socket_set_non_blocking(int sfd)
{
    int flags, s;

    flags = fcntl(sfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;
    s = fcntl(sfd, F_SETFL, flags);
    if (s == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

static int socket_set_reusable(int sfd) {
    int opt = 1;
    int flags = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (flags < 0) {
        perror("setsockopt");
        return -1;
    }

    return 0;
}

static int socket_create_and_bind(char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int s, sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        perror("getaddrinfo");
        return -1;
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;

        s = bind(sfd, rp->ai_addr, rp->ai_addrlen);
        if (s == 0) {
            /* We managed to bind successfully! */
            break;
        }

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not bind\n");
        return -1;
    }

    freeaddrinfo(result);

    return sfd;
}

int main(int argc, char *argv[])
{
    /* listening socket fd */
    int sfd;
    /* epoll fd */
    int efd;
    struct epoll_event event;
    struct epoll_event *events;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    sfd = socket_create_and_bind(argv[1]);
    if (sfd == -1)
        abort();

    if (socket_set_non_blocking(sfd) < 0) 
        abort();

    if (socket_set_reusable(sfd) < 0) 
        abort();

    if (listen(sfd, 1) < 0) {
        perror("listen");
        abort();
    }

    // 尝试改大ulimit最大打开文件数，失败报错但不退出
    if (ulimit_set_max_open_files(MAX_OPEN_FILES)) {
        fprintf(stderr, "Cannot set ulimit max open files");
    }

    efd = epoll_create1(0);
    if (efd == -1) {
        perror("epoll_create");
        abort();
    }

    event.data.fd = sfd;
    event.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event) < 0) {
        perror("epoll_ctl");
        abort();
    }

    /* Buffer where events are returned */
    events = calloc(MAX_EVENTS, sizeof event);

    /* The event loop */
    while (1) {
        int n, i;

        n = epoll_wait(efd, events, MAX_EVENTS, -1);
        for (i = 0; i < n; i++) {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)
                || (!(events[i].events & EPOLLIN))) {
                /* An error has occured on this fd, or the socket is not
                   ready for reading (why were we notified then?) */
                perror("epoll_wait");
                close(events[i].data.fd);
                continue;
            }

            else if (sfd == events[i].data.fd) {
                /* We have a notification on the listening socket, which
                   means one or more incoming connections. */
                while (1) {
                    struct sockaddr in_addr;
                    socklen_t in_len;
                    int infd;
                    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                    in_len = sizeof in_addr;
                    infd = accept(sfd, &in_addr, &in_len);
                    if (infd == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /* We have processed all incoming
                               connections. */
                            break;
                        } else {
                            perror("accept");
                            break;
                        }
                    }

                    {
                        struct linger so_linger;

                        so_linger.l_onoff = 0;
                        so_linger.l_linger = 0;
                        setsockopt(infd,
                                   SOL_SOCKET, SO_LINGER, &so_linger,
                                   sizeof so_linger);
                    }

                    getnameinfo(&in_addr, in_len,
                                hbuf, sizeof hbuf,
                                sbuf, sizeof sbuf,
                                NI_NUMERICHOST | NI_NUMERICSERV);
                    /*
                       if (s == 0)
                       {
                       printf("Accepted connection on descriptor %d "
                       "(host=%s, port=%s)\n", infd, hbuf, sbuf);
                       }
                     */

                    /* Make the incoming socket non-blocking and add it to the
                       list of fds to monitor. */
                    if (socket_set_non_blocking(infd) < 0)
                        abort();

                    event.data.fd = infd;
                    event.events = EPOLLIN | EPOLLET;
                    if (epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event) < 0) {
                        perror("epoll_ctl");
                        abort();
                    }
                }
                continue;
            } else {
                /* We have data on the fd waiting to be read. Read and
                   display it. We must read whatever data is available
                   completely, as we are running in edge-triggered mode
                   and won't get a notification again for the same
                   data. */
                int done = 0;

                while (1) {
                    ssize_t count;
                    char buf[512];

                    count = read(events[i].data.fd, buf, sizeof buf);
                    //printf("read count = %ld, error = %d\n", count, errno);
                    if (count == -1) {
                        /* If errno == EAGAIN, that means we have read all
                           data. So go back to the main loop. */
                        if (errno != EAGAIN) {
                            perror("read");
                        }
                        done = 1;
                        break;
                    } else if (count == 0) {
                        /* End of file. The remote has closed the
                           connection. */
                        done = 1;
                        break;
                    }

                    /* Write the buffer to standard output
                       s = write (1, buf, count);
                       if (s == -1)
                       {
                       perror ("write");
                       abort ();
                       } */
                }

                if (done) {
                    const char *msg =
                        "HTTP/1.1 200 OK\r\n\r\n<html><h1>Hello epoll!</h1></html>";
                    size_t w = write(events[i].data.fd, msg, strlen(msg));

                    //printf("write fd = %d, w = %ld\n", events[i].data.fd, w);
                    /* Closing the descriptor will make epoll remove it
                       from the set of descriptors which are monitored. */
                    epoll_ctl(efd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    close(events[i].data.fd);
                }
            }
        }
    }

    free(events);

    close(sfd);

    return EXIT_SUCCESS;
}
