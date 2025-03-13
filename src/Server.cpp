/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 20:44:56 by vmassoli          #+#    #+#             */
/*   Updated: 2025/02/26 14:04:06 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"

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
    : is_running(true), port(port), password(password), connexionCommands()
{
    connexionCommands.insert("PASS");
    connexionCommands.insert("NICK");
    connexionCommands.insert("USER");
    connexionCommands.insert("CAP");
    connexionCommands.insert("PING");
    connexionCommands.insert("PRIVMSG");
    connexionCommands.insert("WHO");

    Server::instance = this;

    signal(SIGINT, Server::signal_handler);
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
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cerr << COLOR_RED << "Erreur lors de la création de l'epoll." << COLOR_RESET << std::endl;
        return 1;
    }

    struct epoll_event ev, events[120];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        std::cerr << COLOR_RED << "Erreur lors de l'ajout du serveur à l'epoll." << COLOR_RESET << std::endl;
        close(epoll_fd);
        return 1;
    }

    while (is_running)
    {
        int nfds = epoll_wait(epoll_fd, events, 120, -1);
        if (nfds == -1) {
            std::cerr << COLOR_RED << "Erreur dans epoll_wait()." << COLOR_RESET << std::endl;
            break;
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == server_fd)
            {
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                if (client_fd == -1) {
                    std::cerr << COLOR_RED << "Erreur lors de l'acceptation d'une connexion." << COLOR_RESET << std::endl;
                    continue;
                }

                int flags = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                std::cout << COLOR_YELLOW << "[AUTH] Connexion en attente du mot de passe pour FD: " << client_fd << COLOR_RESET << std::endl;
                
                // Ajoute le client à la liste des non-authentifiés
                waitingForPass[client_fd] = true;

                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
                    std::cerr << COLOR_RED << "Erreur lors de l'ajout du client à l'epoll." << COLOR_RESET << std::endl;
                    close(client_fd);
                    continue;
                }
            }
            else
            {
                int fd = events[n].data.fd;
                char buffer[1024];
                int bytes_read = recv(fd, buffer, sizeof(buffer) - 1, 0);
                if (bytes_read > 0)
                {
                    buffer[bytes_read] = '\0';
                    // std::cout << " Message reçu du client " << fd << ": " << buffer << std::endl;

                    std::istringstream stream(buffer);
                    std::string line;
                    while (std::getline(stream, line))
                    {
                        if (!line.empty())
                        {
                            std::istringstream iss(line);
                            std::string command, receivedPassword;
                            iss >> command >> receivedPassword;

                            // **Attente de PASS obligatoire**
                            if (waitingForPass.find(fd) != waitingForPass.end())
                            {
                                if (command == "PASS")
                                {
                                    if (receivedPassword == this->password)
                                    {
                                        std::cout << COLOR_GREEN << "[AUTH] Mot de passe valide pour FD: " << fd << COLOR_RESET << std::endl;
                                        waitingForPass.erase(fd); // **Retirer de la liste des non-authentifiés**
                                        addClient(fd, "", "");
                                        sendMessage(fd, ":server NOTICE * :Password accepted, proceed with NICK and USER\r\n");
                                    }
                                    else
                                    {
                                        std::cerr << COLOR_RED << "[SECURITY] Connexion refusée pour FD: " << fd << " (Mot de passe incorrect)" << COLOR_RESET << std::endl;
                                        sendError(fd, "464", "PASS", "Password incorrect");
                                        close(fd);
                                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                                        waitingForPass.erase(fd); // 🔥 Nettoyer la mémoire après échec d'authentification
                                    }
                                }
                                else
                                {
                                    // 🔥 **Le client envoie autre chose que PASS, on ignore mais on ne ferme pas**
                                    std::cerr << COLOR_YELLOW << "[SECURITY] FD " << fd << " doit encore envoyer PASS" << COLOR_RESET << std::endl;
                                }
                            }
                            else
                            {
                                parseCommand(line, fd);
                            }
                        }
                    }
                }
                else if (bytes_read == 0)
                {
                    std::cout << "Client déconnecté (FD: " << fd << ")." << std::endl;
                    waitingForPass.erase(fd); // 🔥 Supprime le client de la liste des non-authentifiés
                    removeClient(fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                }
                else
                {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        std::cerr << COLOR_RED << "Erreur lors de la réception des données du client (FD: " << fd << ")." << COLOR_RESET << std::endl;
                        waitingForPass.erase(fd);
                        removeClient(fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        close(fd);
                    }
                }
            }
        }
    }

    close(epoll_fd);
    close_all_clients();
    close(server_fd);
    return 0;
}


void Server::addClient(int client_fd, const std::string &name, const std::string &nickname)
{
    std::string host = getClientHost(client_fd); // 🔥 Récupère l'IP du client
	
    if (host.empty())
        host = "127.0.0.1";  // 🔥 Si aucun hostname valide, utiliser l'IP locale

    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getFd() == client_fd) // Vérifier si le FD existe déjà
        {
            std::cerr << "[ERROR] Trying to add an already existing FD: " << client_fd << std::endl;
            return;
        }
    }

    // ✅ Création du client avec l'IP récupérée
    Client* newClient = new Client(client_fd, name, nickname, host);
    clients.push_back(newClient);

    std::cout << "Client added : FD = " << client_fd
              << ", Name = " << name
              << ", Nickname = " << nickname
              << ", Host = " << host << std::endl;
}


void Server::broadcastToChannels(int client_fd, const std::string& message)
{
	// std::cout << " Broadcasting message to channels for FD " << client_fd << ": " << message;
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

std::string Server::getClientHost(int client_fd)
{
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    // 🔎 Obtenir l'adresse IP du client
    if (getpeername(client_fd, (struct sockaddr*)&addr, &addr_len) == -1) {
        std::cerr << "[ERROR] getpeername() a échoué pour FD " << client_fd << std::endl;
        return "unknown";
    }

    // 🔎 Convertir l'adresse IP en string et retourner directement
    return std::string(inet_ntoa(addr.sin_addr));
}



void Server::removeClient(int fd)
{
    std::string nickname = getNickname(fd);

    // Trouver le client dans `clients`
    std::vector<Client*>::iterator it;
    for (it = clients.begin(); it != clients.end(); ++it)
    {
        if ((*it)->getFd() == fd)
            break;
    }

    // Vérifier si on a trouvé le client
    if (it != clients.end()) 
    {
        // std::cout << " 🔥 Suppression du client FD " << fd << " (" << nickname << ")\n";

        // Supprimer le client de tous les channels
        for (size_t i = 0; i < channels.size(); ++i) 
        {
            if (channels[i]->isUserInChannel(nickname))  
            {
                channels[i]->removeUser(fd); // ✅ Correction ici (fd au lieu de nickname)
                // std::cout << " ❌ FD " << fd << " retiré du channel " << channels[i]->getName() << "\n";
            }
        }

        // Supprimer de `clients`
        delete *it;  // Libère la mémoire
        clients.erase(it);  // Supprime du `vector`

        // std::cout << " ✅ Client FD " << fd << " supprimé avec succès !\n";
    }
    else
    {
        // std::cout << " ⚠️ Tentative de suppression d'un client FD " << fd << " qui n'existe pas.\n";
    }
}
