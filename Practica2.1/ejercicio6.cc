#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NUM_THREADS 5

class MessageThread{
    public:
    MessageThread(int s):sd(s){};
    virtual ~MessageThread(){};
    void do_message(){
        while(true){
            char buff[256];
            char host[NI_MAXHOST];
            char serv[NI_MAXSERV];
            
            struct sockaddr client;
            socklen_t clientlen = sizeof(client);

            ssize_t nbytes = recvfrom(sd, buff, 255, 0, &client, &clientlen);
            getnameinfo(&client, clientlen, host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST);
            std::cout << nbytes << " bytes de " << host << " " << serv << " Thread: " << pthread_self() << "\n";

            char buffer[256];
            memset((void*) buffer, '\0', 256);
            time_t rawtime;
            time_t date;
            struct tm *infot;

            switch(buff[0]){
                case 'd':
                time(&date);
                infot = localtime(&date);
                bytes = strftime(buffer, 80, "%F", infot);
                buffer[bytes] = '\0';
                sendto(sd, buffer, 256, 0, &client, clientlen);
                break;
                case 't':
                time(&rawtime);
                infot = localtime(&rawtime);
                bytes = strftime(buffer, 80, "%r", infot);
                buffer[bytes] = '\0';
                sendto(sd, buffer, 256, 0, &client, clientlen);
                break;
                case 'q':
                std::cout << "Para salir hay que poner q este terminal" << std::endl;
                break;
                default:
                std::cout << "Comando no soportado " << buff[0] << std::endl;
                break;
            }
            sleep(3);
        }
    }
    private:
    size_t bytes;
    int sd;
};

extern "C" void *_do_message (void* _mt){
    MessageThread * mt = static_cast<MessageThread*>(_mt);
    mt->do_message();
    delete mt;
    return 0;
}

int main (int argc, char **argv){
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, '\0', sizeof(struct addrinfo));
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
    listen(sd, 15);
    freeaddrinfo(result);

    for(int i = 0; i <= NUM_THREADS; i++){
        pthread_t tid;
        pthread_attr_t attr;

        MessageThread* mt = new MessageThread(sd);

        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        pthread_create(&tid, &attr, _do_message, static_cast<void*>(mt));
    }

    char c;
    while(c != 'q'){
        std::cin >> c;
    }

    freeaddrinfo(result);
    close(sd);

    return 0;
}