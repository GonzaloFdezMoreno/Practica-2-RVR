#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

void procesaMensaje(int sd, char command, struct sockaddr* addr, socklen_t addrlen, bool& quit, char *msg){
    time_t t;
    char infoTime[80];
    size_t bytes;

    switch(command){
        case 't':
        time(&t);
        bytes = strftime(infoTime, sizeof(infoTime), "%T", localtime(&t));
        sendto(sd, infoTime, bytes, 0, addr, addrlen);
        break;
        case 'd':
        time(&t);
        bytes = strftime(infoTime, sizeof(infoTime), "%F", localtime(&t));
        sendto(sd, infoTime, bytes, 0, addr, addrlen);
        break;
        case 'q':
        quit = true;
        strcpy(msg, "Saliendo...");
        sendto(sd, "", 0, 0, addr, addrlen);
        break;
        default: 
        strcpy(msg, "Comando no soportado"); //no hemos encontrado la manera de meter command en el mensaje
        sendto(sd, "", 0, 1, addr, addrlen);
        break;
    }
}

int main(int argc, char *argv[]){
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
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

    bool quit = false;
    while(!quit){
        char buffer[80];
        char host[NI_MAXHOST];
        char serv[NI_MAXSERV];

        struct sockaddr client;
        socklen_t clientlen = sizeof(struct sockaddr);
        char msg[80] = "";

        ssize_t bytes = recvfrom(sd, buffer, 79, 0, (struct sockaddr*) &client, &clientlen);
        buffer[bytes] = '\0';

        getnameinfo(&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        procesaMensaje(sd, buffer[0], &client, clientlen, quit, msg);

        std::cout << bytes << " bytes de " << host << ":" << serv << '\n';
        std::cout << msg << '\n';
    }
    freeaddrinfo(result);
    return 0;
}