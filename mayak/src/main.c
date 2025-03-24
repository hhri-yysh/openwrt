#include "../lib/icmp.h"
#include "../lib/socket.h"
#include "../lib/un.h"

void
err_quit (const char *msg) {
        perror (msg);
        exit (0);
}

void
err_sys (const char *msg) {
        perror (msg);
        exit (0);
}

int
main (int argc, char **argv) {
        /*
         *	Example of working with glibc (getopt.h):
         *	https://www.gnu.org/software/libc/manual/html_node/Getopt-Long-Option-Example.html
         *
         */

        int sockfd;
        int opt, timeset = TIMEOUT;
        int max_hop = MAX_HOP;
        int ip_ver = AF_INET;
        int fqdn_fl = 0;

        struct sockaddr_in sender;
        struct sockaddr_in target;
        struct timeval start_time;

        socket_work (&sockfd);

        while (1) {
                static struct option long_opt[]
                    = { { "help", no_argument, 0, 'h' },
                        { "ipv4", no_argument, 0, '4' },
                        { "ipv6", no_argument, 0, '6' },
                        { "fqdn", no_argument, 0, 'f' },
                        { "timeout", required_argument, 0, 't' },
                        { "max-hop", required_argument, 0, 'm' },
                        { 0, 0, 0, 0 } };

                int opt_index = 0;

                opt = getopt_long (argc, argv, "46fht:m:", long_opt,
                                   &opt_index);
                if (opt == -1)
                        break;
                switch (opt) {
                case 0:
                        if (long_opt[opt_index].flag != 0)
                                break;
                        printf ("%s", long_opt[opt_index].name);
                        if (optarg)
                                printf ("%s", optarg);
                        printf ("\n");
                        break;
                case 'h':
                        printf ("\t --ipv4, -4\n");
                        printf ("\t --ipv6, -6\n");
                        printf ("\t --fqdn, -f\n");
                        printf ("\t --max-hop, -m\n");
                        printf ("\t --timeout, -t\n");
                        break;
                case '4':
                        ip_ver = AF_INET;
                        printf ("IPv4 select\n");
                        break;
                case '6':
                        ip_ver = AF_INET6;
                        printf ("IPv6 select\n");
                        break;
                case 'f':
                        fqdn_fl = 1;
                        break;
                case 't':
                        timeset = atoi (optarg);
                        if (timeset < 0) {
                                fprintf (stderr, "Error, wrong timeout\n");
                                exit (EXIT_FAILURE);
                        }
                        break;
                case 'm':
                        max_hop = atoi (optarg);
                        if (max_hop < 0) {
                                fprintf (stderr, "Wrong max-hop\n");
                                exit (EXIT_FAILURE);
                        }
                        break;
                case '?':

                        break;
                default:
                        abort ();
                }
        }

        if (optind >= argc) {
                fprintf (stderr, "Usage: %s [OPTION] <ip>\n", argv[0]);
                exit (EXIT_FAILURE);
        }

        const char *target_input = argv[optind];
        char target_ip [INET6_ADDRSTRLEN];
        struct addrinfo hints, *res;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = ip_ver; // AF_INET or AF_INET6
        hints.ai_socktype = SOCK_RAW;
        // Domain resolver: ./mayak dns.google.com -> 8.8.8.8
        // Pass hints to getaddrinfo
        if (getaddrinfo(target_input, NULL, &hints, &res) != 0) {
                fprintf(stderr, "Error resolving domain name: %s\n", target_input);
                exit(EXIT_FAILURE);
        }
        // Convert domain address to string
        if (res->ai_family == AF_INET) {
                struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
                inet_ntop(AF_INET, &addr->sin_addr, target_ip, sizeof(target_ip));
            } else if (res->ai_family == AF_INET6) {
                struct sockaddr_in6 *addr = (struct sockaddr_in6 *)res->ai_addr;
                inet_ntop(AF_INET6, &addr->sin6_addr, target_ip, sizeof(target_ip));
            }

        // if (ip_ver == AF_INET) {
        //         struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
        //         inet_ntop(AF_INET, &addr->sin_addr, target_ip, sizeof(target_ip));
        // } else if (ip_ver == AF_INET6) {
        //         struct sockaddr_in6 *addr = (struct sockaddr_in6 *)res->ai_addr;
        //         inet_ntop(AF_INET6, &addr->sin6_addr, target_ip, sizeof(target_ip));
        // }
        
        freeaddrinfo(res);

        printf ("Trace to %s (%s)\n", target_input, target_ip);

        for (int ttl = 1; ttl <= max_hop; ttl++) {
                memset (&target, 0, sizeof (target));
                target.sin_family = ip_ver;

                if (ip_ver == AF_INET) {
                        if (inet_pton (AF_INET, target_ip, &target.sin_addr)
                            <= 0) {
                                perror ("inet_pton error");
                                exit (EXIT_FAILURE);
                        } else if (ip_ver == AF_INET6) {
                                struct sockaddr_in6 target6;
                                memset (&target6, 0, sizeof (target6));
                                target6.sin6_family = AF_INET6;
                                if (inet_pton (AF_INET6, target_ip,
                                               &target6.sin6_addr)
                                    <= 0) {
                                        perror ("inet_pton error");
                                        exit (EXIT_FAILURE);
                                }
                        }
                }

                if (send_echo_req (sockfd, &target, getpid (), ttl, ttl,
                                   &start_time)
                    < 0) {
                        fprintf (stderr, "Failed to send packet\n");
                        continue;
                }

                int status = recv_echo_reply (sockfd, ttl, &start_time, timeset,
                                              sender, fqdn_fl);
                if (status < 0) {
                        fprintf (stderr, "Failed to receive packet\n");
                        continue;
                }
                usleep(50000);
        }
}
