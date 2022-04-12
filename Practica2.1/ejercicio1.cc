#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

int main(int argc, char **argv){
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;

    int rc = getaddrinfo(argv[1], argv[2], &hints, &result);

    if (rc != 0){
        std::cerr << "Error getaddrinfo: " << gai_strerror(rc) << std::endl;
        return -1;
    }

    for(struct addrinfo *i = result; i != nullptr; i = i->ai_next){
        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];

        int getnameinfo(i->ai_addr, i->ai_addrlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        
        std::cout << "HOST: " << host << "\t PORT: " << serv << std::endl;

    }
    freeaddrinfo(result);

    return 0;
}

