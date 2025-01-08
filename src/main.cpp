#include "Server.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return (1);
    }
    ft_irc();
    return (0);
}

void ft_irc()
{
    Server server;

    if (server.CreateSocket())
    {
        // Bind the socket to the port
    }
    // Listen for incoming connections
    // Accept a connection
    // Receive data
    // Send data
    // Close the connection
    // Close the socket
}
