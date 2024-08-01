#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>

#include "addr.h"

int localAddr(Node** head)
{
    int sockfd;
    struct ifconf ifconf;
    struct ifreq *ifreq;
    char buf[512];

    //初始化ifconf
    ifconf.ifc_len = sizeof(buf);
    ifconf.ifc_buf = buf;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    //获取所有接口信息
    ioctl(sockfd, SIOCGIFCONF, &ifconf);

    //接下来一个一个的获取IP地址
    ifreq = (struct ifreq *) ifconf.ifc_buf;
    int ndevs = ifconf.ifc_len / sizeof(struct ifreq);

    for (int i = 0; i < ndevs; i++) {
        if (ifreq->ifr_flags == AF_INET) {  //for ipv4
            const char* addr = inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
            addNode(head, ifreq->ifr_name, addr);
            //printf("name =[%s]\n", ifreq->ifr_name);
            //printf("local addr = [%s]\n", addr);
            ifreq++;
        }
    }
    return ndevs;
}
