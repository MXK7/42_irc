#include "../includes/Server.hpp"

Server::Server(int port, const std::string& password)
{
	this->port = port;
	this->password = password;
}

/**
 * @brief Create a server socket

This function initializes a server socket using the IPv4 protocol (AF_INET) and TCP (SOCK_STREAM). 
It also sets the `SO_REUSEADDR` option, which allows the server to reuse the address without waiting 
for the operating system to release the port.

 * @return true if the socket was created and configured successfully, false otherwise.

 * @socket: 
	- AF_INET = IPv4
	- SOCK_STREAM = Reliable and ordered communication (TCP)
	- 0 = Automatically selects the appropriate protocol (TCP for SOCK_STREAM, UDP for SOCK_DGRAM)

 * @setsockopt: 
	- tmp_opt = 1: Enables the reuse option.
	- SOL_SOCKET, SO_REUSEADDR: Allows the address to be reused.

 * @sockaddr_in:
	struct sockaddr_in {
		sa_family_t    sin_family;   // AF_INET (IPv4)
		uint16_t       sin_port;     // Port (big-endian grâce à htons)
		struct in_addr sin_addr;     // Adresse IP (ici INADDR_ANY)
		char           sin_zero[8];  // Non utilisé (rempli par memset)
	};

 * @INADDR_ANY:
	Authorize all connections on all local interfaces.

 * @htons:
	The 16-bit integer (short) to be converted, represented in host byte order.

 * @errors: 
	- Prints ERR_CR_SOCK if socket creation fails.
	- Prints ERR_CFG_SOCK and exits the program if configuration fails.
 */
int Server::CreateSocket()
{
	this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << COLOR_RED << ERR_CR_SOCK << std::endl;
		return (-1);
	}

	int tmp_opt = 1;
	if (setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR, &tmp_opt, sizeof(tmp_opt)) < 0)
	{
		std::cerr << COLOR_RED << ERR_CFG_SOCK << std::endl;
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if(bind(this->server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)//bind the socketsto the address and port
	{
		std::cerr << COLOR_RED << ERR_BIND_SOCK << COLOR_RESET << std::endl;
		close(this->server_fd);
		return(-1);
	}

	if(listen(this->server_fd, SOMAXCONN) < 0) //socket on 
	{
		std::cerr << COLOR_RED << ERR_LISTEN_SOCK << COLOR_RESET << std::endl;
		close(this->server_fd);
	}

	if (fcntl(this->server_fd, F_SETFL, O_NONBLOCK) < 0) {
		std::cerr << ERR_FCNTL_SOCK << std::endl;
		close(this->server_fd);
	}

	std::cout << COLOR_GREEN << "SOCKET CREATED" << COLOR_RESET << std::endl;
	std::cout << COLOR_BLUE << "Password : " << COLOR_RESET << password << std::endl;
	std::cout << COLOR_BLUE << "Port : " << COLOR_RESET << port << std::endl;

	return (0);
}

int Server::HandlerConnexion()
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd;

    std::cout << "Serveur en attente de connexions..." << std::endl;

    while (true)
    {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // Pas de connexion disponible pour le moment, continue la boucle
                continue;
            }
            std::cerr << COLOR_RED << "Erreur lors de l'acceptation d'une connexion." << COLOR_RESET << std::endl;
            continue;
        }

        std::cout << COLOR_GREEN << "Nouvelle connexion acceptée." << COLOR_RESET << std::endl;

        char buffer[1024];
        int bytes_read;

        bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0)
        {
            buffer[bytes_read] = '\0';
            std::cout << "Message reçu du client : " << buffer << std::endl;

            const char* welcome_message = "Welcome to this IRC server!";
            send(client_fd, welcome_message, strlen(welcome_message), 0);
        }
        else if (bytes_read == 0)
        {
            std::cout << "Client déconnecté." << std::endl;
        }
        else
        {
            std::cerr << COLOR_RED << "Erreur lors de la réception des données du client." << COLOR_RESET << std::endl;
        }
        close(client_fd);
    }
    return close(server_fd);
}
