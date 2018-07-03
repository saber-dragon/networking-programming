#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 500

int main(int argc, char *argv[]) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    ssize_t nread;
    char buf[BUF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s address port\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char *address = argv[1];

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
    hints.ai_protocol = 0;           /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully bind(2).
       If socket(2) (or bind(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
            sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            printf("create socket ...\n");
        } else
            continue;
        if (sfd == -1) continue;

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0) break; /* Success */

        close(sfd);
    }

    int on = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        fprintf(stderr, "Setsockopt failed\n");
        exit(EXIT_FAILURE);
    }
    listen(sfd, 3);

    if (rp == NULL) { /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(result); /* No longer needed */

    /* Read datagrams and echo them back to sender */

    for (;;) {
        int cfd = accept(sfd, (struct sockaddr *)&peer_addr, &peer_addr_len);
        if (cfd < 0) {
            fprintf(stderr, "Accept failed\n");
        }
        peer_addr_len = sizeof(struct sockaddr_storage);
        while (1) {
            nread = recv(cfd, buf, BUF_SIZE, 0);
            if (nread == -1) continue; /* Ignore failed request */

            char host[NI_MAXHOST], service[NI_MAXSERV];

            s = getnameinfo((struct sockaddr *)&peer_addr, peer_addr_len, host,
                            NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICSERV);
            if (s == 0)
                printf("Received %s (%zd bytes) from %s:%s\n", buf, nread, host,
                       service);
            else
                fprintf(stderr, "getnameinfo: %s\n", gai_strerror(s));

            if (send(cfd, buf, nread, 0) != nread)
                fprintf(stderr, "Error sending response\n");
            int error = 0;
            socklen_t len = sizeof(error);
            int retval = getsockopt(cfd, SOL_SOCKET, SO_ERROR, &error, &len);
            if (retval != 0) {
                fprintf(stderr, "Cannot getsockopt\n");
                close(cfd);
                exit(EXIT_FAILURE);
            }
            if (error != 0) {
                fprintf(stderr, "Sock error: %s\n", strerror(error));
                close(cfd);
                break;
            }
        }
    }
    close(sfd);
}
