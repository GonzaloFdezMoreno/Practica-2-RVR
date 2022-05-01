#include "Chat.h"
#include <memory.h>

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
    alloc_data(MESSAGE_SIZE);

    memset(_data, 0, MESSAGE_SIZE);

    //Serializar los campos type, nick y message en el buffer _data
    char *temp = _data;

    memcpy(temp, &type, sizeof(uint8_t));
    temp += sizeof(uint8_t);

    memcpy(temp, nick.c_str(), sizeof(char) * 8);
    temp += sizeof(char) * 8;

    memcpy(temp, message.c_str(), sizeof(char) * 80);
}

int ChatMessage::from_bin(char * bobj)
{
    alloc_data(MESSAGE_SIZE);

    memcpy(static_cast<void *>(_data), bobj, MESSAGE_SIZE);

    //Reconstruir la clase usando el buffer _data
    char *temp = _data;

    memcpy(&type, temp, sizeof(uint8_t));
    temp += sizeof(uint8_t);

    nick = temp;
    temp += sizeof(char) * 8;

    message = temp;

    return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
    while (true)
    {
        /*
         * NOTA: los clientes están definidos con "smart pointers", es necesario
         * crear un unique_ptr con el objeto socket recibido y usar std::move
         * para añadirlo al vector
         */
        
        ChatMessage chatmess;
        Socket * sock;

        socket.recv(chatmess, sock);

        //Recibir Mensajes en y en función del tipo de mensaje
        // - LOGIN: Añadir al vector clients
        // - LOGOUT: Eliminar del vector clients
        // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
        int counter = 0;
        switch(chatmess.type) {
            case ChatMessage::LOGIN:
                std::cout << chatmess.nick << " connected" << std::endl;
                //Este da problemas con el unique pointer porque std no tiene make_unique aqui y no logramos resolverlo
                clients.push_back(std::move(std::unique_ptr<Socket>(sock)));
            break;
            case ChatMessage::LOGOUT:
                auto it = clients.begin();
                while(it != clients.end() && (**it != *sock)){
                    it++;
                }
                if(it == clients.end()){
                    std::cout << "The player had already disconnected" << std::endl;
                }
                else{
                    std::cout << chatmess.nick << " disconnected" << std::endl;
                    clients.erase(it);
                    Socket *socktodel = (*it).release();
                    delete socktodel;
                }
            break;
            case ChatMessage::MESSAGE:
                for(auto it = clients.begin(); it != clients.end(); it++){
                    if(**it != *sock){
                        socket.send(chatmess, **it);
                    }
                }
            break;
            default:
                std::cout << "Error unknown message received" << std::endl;
            break;
        }

    }
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGIN;

    socket.send(em, socket);
}

void ChatClient::logout()
{
    // Completar
    std::string msg;

    ChatMessage em(nick, msg);
    em.type = ChatMessage::LOGOUT;

    socket.send(em, socket);
}

void ChatClient::input_thread()
{
    while (true)
    {
        // Leer stdin con std::getline
        // Enviar al servidor usando socket
        std::string msg;
        std::getline(std::cin, msg);

        if (msg.size() > 80) msg.resize(80);

        ChatMessage em(nick, msg);
        em.type = ChatMessage::MESSAGE;

        socket.send(em, socket);
    }
}

void ChatClient::net_thread()
{
    while(true)
    {
        //Recibir Mensajes de red
        //Mostrar en pantalla el mensaje de la forma "nick: mensaje"
        ChatMessage em;
        socket.recv(em);
        std::cout << em.nick << ": " << em.message << std::endl;
    }
}

