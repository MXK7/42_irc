#include "Server.hpp"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return (1);
    }

    int port = atoi(argv[1]);
    std::string password = argv[2];

    if (port <= 0 || port > 65535)
    {
        std::cerr << "Erreur : Le port doit être un entier valide entre 1 et 65535." << std::endl;
        return 1;
    }

    Server server(port, password);

    if (server.CreateSocket() == 0) // Si CreateSocket retourne 0, on lance HandlerConnexion
        server.HandlerConnexion();
    else
    {
        std::cerr << "Erreur lors de la création du serveur." << std::endl;
        return (1);
    }

    // close(server_fd);
    return (0);
}
