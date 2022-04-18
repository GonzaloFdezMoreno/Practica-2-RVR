#include <iostream>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

int main(int argc, char *argv[]){
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    int rc = getaddrinfo(argv[1], argv[2], &hints, &result);

    if(rc != 0) {
        std::cerr << "Error getaddrinfo: " << gai_strerror(rc) << std::endl;
        return -1;
    }

    int sd = socket(result->ai_family, result->ai_socktype, 0);
    if(sd == -1){
        std::cerr << "Error socket: " << gai_strerror(sd) << std::endl;
        return -1;
    }

    bind(sd, result->ai_addr, result->ai_addrlen);

    struct sockaddr source_addr;
    socklen_t sourcelen = sizeof(struct sockaddr);
    char buffer[80];

    source_addr = *result->ai_addr;
    strcpy(buffer, argv[3]);

    sendto(sd, buffer, sizeof(buffer), 0, &source_addr, sourcelen);

    ssize_t bytes = recvfrom(sd, buffer, sizeof(buffer), 0, &source_addr, &sourcelen);
    buffer[bytes] = '\0';
    
    std::cout << buffer << '\n';

    return 0;
}