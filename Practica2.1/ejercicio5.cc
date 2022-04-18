#include <netdb.h>
#include <iostream>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

const size_t MESSAGE_MAX_SIZE = 100;

int main(int argc, char **argv)
{
    
    if (argc != 3)
    {
        //en el caso de poner mas parametros de la cuenta
        std::cerr << "Nº de parámetros incorrectos\n Formato: .\ejercicio_5 <direccion> <puerto>\n ";
        return -1;
    }

    struct addrinfo hints;
    struct addrinfo *res;

    memset((void *)&hints, 0, sizeof(struct addrinfo));

    hints.ai_flags = AI_PASSIVE;     
    hints.ai_family = AF_INET;       
    hints.ai_socktype = SOCK_STREAM;

    int recieve = getaddrinfo(argv[1], argv[2], &hints, &res);
    int sd = socket(res->ai_family, res->ai_socktype, 0);
    int sd_server = connect(sd, (struct sockaddr *)res->ai_addr, res->ai_addrlen);

    //Gestion de errores

    if (recieve != 0)
    {
        std::cerr << "Error getaddrinfo: " << gai_strerror(recieve) << std::endl;
        return -1;
    }

    if (sd == -1)
    {
        std::cerr << "Error en la creación de [socket]\n";
        return -1;
    }

    /*if (bind(sd, res->ai_addr, res->ai_addrlen) == -1)
    {
        std::cerr << "Error en la llamada a [bind]\n";
        return -1;
    }*/


    if (sd_server == -1)
    {
        std::cerr << "No es posible conectarse al servidor\n";
        return -1;
    }

   freeaddrinfo(res);

    /*char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    struct sockaddr client;
    socklen_t clientLen = sizeof(struct sockaddr);
    int client_sd = accept(sd, (struct sockaddr *)&client, &clientLen);

    if(client_sd == -1){
        std::cerr << "No se ha aceptado la conexion TCP con el cliente [accept]\n";
    } 

    getnameinfo((struct sockaddr *)&client, clientLen, host, NI_MAXHOST, serv,
                 NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
    std::cout<<"Conexión desde:"<<host<<serv<<'\n';*/

    char buffer[MESSAGE_MAX_SIZE];

    bool serverActive = true;
    while (serverActive) 
    {
        //char buffer[MESSAGE_MAX_SIZE];
        std::cin >> buffer;

        ssize_t bytesSended = send(sd_server, (void *) buffer, (MESSAGE_MAX_SIZE -1 )* sizeof(char), 0);

        //salir si se presiona Q y comprobamos que unicamente ha sido ese caracter enviado
        if(buffer[0]=='Q'&&buffer[1]=='\0'){
            std::cout << "Conexion terminada\n";
            serverActive = false;
            continue;
        }
        
        recv(sd_server, (void *) buffer, bytesSended, 0);

        std::cout << buffer << '\n';
    }

    close(sd_server);
    close(sd);

    return 0;
}