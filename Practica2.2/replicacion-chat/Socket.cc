#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char * address, const char * port):sd(-1)
{
    //Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
    //Con el resultado inicializar los miembros sd, sa y sa_len de la clase
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int rc = getaddrinfo(address, port, &hints, &result);
    if(rc != 0){
        std::cerr << "Error getaddrinfo: " << gai_strerror(rc) << std::endl;
    }

    sd = socket(result->ai_family, result->ai_socktype, 0);
    if(sd == -1){
        std::cerr << "Error socket: " << gai_strerror(sd) << std::endl;
    }

    sa = *result->ai_addr;
    sa_len = result->ai_addrlen;

    freeaddrinfo(result);
}

int Socket::recv(Serializable &obj, Socket * &sock)
{
    struct sockaddr sa;
    socklen_t sa_len = sizeof(struct sockaddr);

    char buffer[MAX_MESSAGE_SIZE];

    ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);

    if ( bytes <= 0 )
    {
        return -1;
    }

    if ( sock != 0 )
    {
        sock = new Socket(&sa, sa_len);
    }

    obj.from_bin(buffer);

    return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
    //Serializar el objeto
    obj.to_bin();
    //Enviar el objeto binario a sock usando el socket sd

    ssize_t bytessend = sendto(sock.sd, obj.data(), obj.size(), 0, &sock.sa, sock.sa_len);

    if(bytessend <= 0){
        std::cerr << "Error del envio del mensaje" << std::endl;
        return -1;
    }

    return 0;
}

bool operator== (const Socket &s1, const Socket &s2)
{
    //Comparar los campos sin_family, sin_addr.s_addr y sin_port
    //de la estructura sockaddr_in de los Sockets s1 y s2
    //Retornar false si alguno difiere
    
    if(s1.sa.sa_family != s2.sa.sa_family){
        return false;
    }

    struct sockaddr_in *sin1 = (struct sockaddr_in *) &(s1.sa);
    struct sockaddr_in *sin2 = (struct sockaddr_in *) &(s2.sa);

    if(sin1->sin_addr.s_addr != sin2->sin_addr.s_addr){
        return false;
    }
    if(sin1->sin_port != sin2->sin_port){
        return false;
    }

    return true;
};

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    getnameinfo((struct sockaddr *) &(s.sa), s.sa_len, host, NI_MAXHOST, serv,
                NI_MAXSERV, NI_NUMERICHOST);

    os << host << ":" << serv;

    return os;
};

bool operator!= (const Socket &s1, const Socket &s2)
{
    return !(s1 == s2);
};
