#pragma once

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"

#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>  // atoi
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <arpa/inet.h> //-> for inet_ntoa(), htons()
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
// #include <vector> //-> for vector
// #include <poll.h> //-> for poll()
// #include <csignal> //-> for signal()

#define ERR_CR_SOCK "Error : Failed to create a socket."
#define ERR_CFG_SOCK "Error : Cannot configure socket options."
#define ERR_BIND_SOCK  "Error: failed to bind the socket."
#define ERR_LISTEN_SOCK "Error: failed to listen to the socket."
#define ERR_FCNTL_SOCK "Error: Unable to make socket non-blocking"

class Server
{
    private:
        int port;
        int server_fd;
        std::string password;

    public:
        Server(int port, const std::string& password);
        int CreateSocket();
        int HandlerConnexion();
};
