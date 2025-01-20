/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 20:44:56 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/20 14:50:19 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"
// #include <vector>

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


Server* Server::instance = NULL;

Server::~Server() {
	// Détruire les objets client et canal et effectuer tout nettoyage nécessaire
	for (std::vector<Client*>::iterator it = clients.begin(); it != clients.end(); ++it) {
		delete *it;  // Libérer la mémoire allouée pour chaque client
	}
	clients.clear();

	for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it) {
		delete *it;  // Libérer la mémoire allouée pour chaque canal
	}
	channels.clear();

	// Autres opérations de nettoyage, comme fermer le socket du serveur
	close(server_fd);
}

Server::Server(int port, const std::string& password)
	: is_running(true)  // Init flag to make the serv run
{
	this->port = port;
	this->password = password;

	Server::instance = this;// Initialiser le pointeur statique de l'instance

	signal(SIGINT, Server::signal_handler); // Associer le gestionnaire de signal à SIGINT
}

// Gestionnaire de signal pour fermer le serveur proprement
void Server::handle_signal(int signal)
{
	(void)signal;
	std::cout << "Arrêt du serveur..." << std::endl;

	std::string shutdown_message = "Le serveur va fermer. Merci de votre connexion.\n";
	for (size_t i = 0; i < client_fds.size(); ++i)
		send(client_fds[i], shutdown_message.c_str(), shutdown_message.size(), 0);

	close_all_clients();
	close(server_fd);
	exit(0);
}


void Server::signal_handler(int signal)
{
	if (instance)
		instance->handle_signal(signal);
}


void Server::close_all_clients()
{
	for (size_t i = 0; i < client_fds.size(); ++i)
		close(client_fds[i]);
	client_fds.clear();
}

int Server::CreateSocket()
{
	this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		std::cerr << COLOR_RED << ERR_CR_SOCK << std::endl;
		return -1;
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

	if (bind(this->server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		std::cerr << COLOR_RED << ERR_BIND_SOCK << COLOR_RESET << std::endl;
		close(this->server_fd);
		return -1;
	}

	if (listen(this->server_fd, SOMAXCONN) < 0)
	{
		std::cerr << COLOR_RED << ERR_LISTEN_SOCK << COLOR_RESET << std::endl;
		close(this->server_fd);
		return -1;
	}

	std::cout << COLOR_GREEN << "SOCKET CREATED" << COLOR_RESET << std::endl;
	std::cout << COLOR_BLUE << "Password : " << COLOR_RESET << password << std::endl;
	std::cout << COLOR_BLUE << "Port : " << COLOR_RESET << port << std::endl;

	return 0;
}

int Server::HandlerConnexion()
{
    fd_set read_fds, temp_fds;
    FD_ZERO(&read_fds);
    FD_SET(server_fd, &read_fds);

    int max_fd = server_fd;

    while (is_running)
    {
        temp_fds = read_fds;

        // Attendre l'activité sur les sockets
        if (select(max_fd + 1, &temp_fds, NULL, NULL, NULL) < 0)
        {
            std::cerr << COLOR_RED << "Erreur dans select()." << COLOR_RESET << std::endl;
            break;
        }

        for (int fd = 0; fd <= max_fd; ++fd)
        {
            if (FD_ISSET(fd, &temp_fds))
            {
                if (fd == server_fd) // Nouvelle connexion
                {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

                    if (client_fd < 0)
                    {
                        std::cerr << COLOR_RED << "Erreur lors de l'acceptation d'une connexion." << COLOR_RESET << std::endl;
                        continue;
                    }

                    // Ajouter le nouveau client
                    addClient(client_fd, "ClientName", ""); // Pas de pseudonyme par défaut
                    client_fds.push_back(client_fd);
                    FD_SET(client_fd, &read_fds);
                    if (client_fd > max_fd) max_fd = client_fd;

                    std::cout << COLOR_GREEN << "Nouvelle connexion acceptée (FD: " << client_fd << ")." << COLOR_RESET << std::endl;

                    // Envoyer un message de bienvenue initial
                    std::string welcomeMsg = ":server NOTICE * :Welcome to the IRC server!\r\n";
                    send(client_fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
                }
                else // Données provenant d'un client existant
                {
                    char buffer[1024];
                    int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);

                    if (bytes_read > 0) // Données reçues
                    {
                        buffer[bytes_read] = '\0';
                        std::cout << "Message reçu du client " << fd << ": " << buffer << std::endl;

                        // Traiter le message avec le parseur
                        parseCommand(buffer, fd);
                    }
                    else if (bytes_read == 0) // Le client a fermé la connexion
					{
						std::cout << "Client déconnecté (FD: " << fd << ")." << std::endl;

						// Informer les autres clients de la déconnexion
						std::string quitMessage = ":" + getNickname(fd) + " QUIT :Client disconnected.\r\n";
						broadcastToChannels(fd, quitMessage);

						// Supprimer le client
						close(fd);
						FD_CLR(fd, &read_fds);
						client_fds.erase(std::remove(client_fds.begin(), client_fds.end(), fd), client_fds.end());
					}
                    else // Erreur de réception
                    {
                        std::cerr << COLOR_RED << "Erreur lors de la réception des données du client (FD: " << fd << ")." << COLOR_RESET << std::endl;

                        // Fermer proprement en cas d'erreur
                        close(fd);
                        FD_CLR(fd, &read_fds);
                        client_fds.erase(std::remove(client_fds.begin(), client_fds.end(), fd), client_fds.end());
                    }
                }
            }
        }
    }
    close_all_clients();
    close(server_fd);
    return 0;
}

void Server::addClient(int client_fd, const std::string &name,
						const std::string &nickname) {
	Client* newClient= new Client(client_fd, name, nickname);
	clients.push_back(newClient);
	std::cout << "Client added : FD = " << client_fd
			  << ", Name = " << name
			  << ", Nickname = " << nickname << std::endl;
}


void Server::broadcastToChannels(int client_fd, const std::string& message)
{
    // Récupérer le pseudonyme du client qui envoie le message
    std::string senderNickname = getNickname(client_fd);

    // Parcourir tous les canaux
    for (size_t i = 0; i < channels.size(); ++i)
    {
        Channel* channel = channels[i];

        // Vérifier si le client fait partie du canal
        if (channel->isUserInChannel(senderNickname))
        {
            // Diffuser le message à tous les autres utilisateurs dans le canal
            channel->broadcast(message, client_fd);
        }
    }
}