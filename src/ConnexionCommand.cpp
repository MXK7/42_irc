/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnexionCommand.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:26:42 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/26 14:02:50 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::handleNickCommand(std::istringstream& iss, int client_fd)
{
    std::string newNickname;
    iss >> newNickname;
    Client* client = getClientByFd(client_fd);

    if (!client) {
        sendError(client_fd, "401", "NICK", "No such nick");
        return;
    }

    // ✅ Vérification du hostname AVANT d'envoyer la commande à Irssi
    if (client->getHostname().empty()) {
        client->setHostname("127.0.0.1");
        std::cout << "[DEBUG] ⚠️ Hostname vide détecté après NICK. Assignation à 127.0.0.1\n";
    }

    if (newNickname.empty()) {
        sendError(client_fd, "431", "NICK", "No nickname given");
        return;
    }

    if (newNickname.find(' ') != std::string::npos || newNickname.find(',') != std::string::npos) {
        sendError(client_fd, "432", "NICK", "Erroneous nickname");
        return;
    }

    for (size_t i = 0; i < clients.size(); ++i) {
        if (clients[i]->getNickname() == newNickname) {
            sendError(client_fd, "433", "NICK", "Nickname is already in use");
            return;
        }
    }

    std::string oldNickname = client->getNickname();
    client->setNickname(newNickname);
    std::cout << "Client FD " << client_fd << " changed nickname from " 
              << oldNickname << " to " << newNickname << std::endl;

    updateNicknameInChannels(oldNickname, newNickname, client_fd);

    std::string nickResponse = ":" + oldNickname + "!~" + client->getUsername() + "@" + client->getHostname() + 
                               " NICK :" + newNickname + "\r\n";
    sendMessage(client_fd, nickResponse);

    broadcastToChannels(client_fd, nickResponse);
}


void Server::handleUserCommand(std::istringstream& iss, int client_fd)
{
    std::cout << "Commande USER reçue pour le client FD: " << client_fd << std::endl;

    std::string username, hostname, servername, realname;
    iss >> username >> hostname >> servername;
    std::getline(iss, realname);

    std::cout << "Paramètres reçus - Username: " << username 
            << ", Hostname: " << hostname 
            << ", Servername: " << servername 
            << ", Realname: " << realname << std::endl;

    if (!realname.empty() && realname[0] == ':')
        realname = realname.substr(1);

    if (username.empty() || realname.empty())
    {
        std::cout << "Erreur: Paramètres manquants pour USER" << std::endl;
        sendError(client_fd, "461", "USER", "Not enough parameters");
        return;
    }

    bool clientFound = false;
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getFd() == client_fd)
        {
            std::cout << "Client trouvé dans la liste, FD: " << client_fd << std::endl;
            clientFound = true;

            clients[i]->setName(realname);
            clients[i]->setUsername(username);

           std::string clientHost = getClientHost(client_fd);
            std::cout << "Hostname du client avant enregistrement : " << clientHost << std::endl;
            if (clientHost.empty()) {
                clientHost = "127.0.0.1";  // Assignation d'un hostname par défaut
                std::cout << "Hostname vide détecté, assignation de 127.0.0.1" << std::endl;
            }
            clients[i]->setHostname(clientHost);
            if (!clients[i]->getNickname().empty())
            {
                std::cout << "Nickname déjà défini, envoi du message de bienvenue" << std::endl;
                sendWelcomeMessage(client_fd);
            }
            else
            {
                std::cout << "Nickname non défini, en attente de la commande NICK" << std::endl;
            }
            break;
        }
    }

    if (!clientFound)
    {
        std::cout << "Erreur: Client non trouvé dans la liste pour FD: " << client_fd << std::endl;
    }
}
