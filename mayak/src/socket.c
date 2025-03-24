#include "../lib/socket.h"

int
socket_work (int *sockfd) {
        *sockfd = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP);
        if (*sockfd < 0) {
                perror ("socket");
                return -1;
        }

        return 0;
}
