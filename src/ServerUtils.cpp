/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerUtils.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/17 09:35:55 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/26 13:59:48 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::sendWelcomeMessage(int client_fd)
{
	std::ostringstream welcomeMsg;
	welcomeMsg << ":irc.server 001 " << getNickname(client_fd) << " :Welcome to the IRC server!\r\n"
			   << ":irc.server 002 " << getNickname(client_fd) << " :Your host is irc.server, running version 1.0\r\n"
			   << ":irc.server 003 " << getNickname(client_fd) << " :This server was created today\r\n";

	send(client_fd, welcomeMsg.str().c_str(), welcomeMsg.str().size(), 0);
	std::ostringstream whoReply;
	whoReply << ":irc.server 352 " << getNickname(client_fd) 
			<< " #chat * " << getNickname(client_fd) 
			<< " irc.server " << getNickname(client_fd) 
			<< " H :0 " << getNickname(client_fd) << "\r\n";
	sendMessage(client_fd, whoReply.str());

	// 🔥 Message de fin de WHO
	std::ostringstream endOfWho;
	endOfWho << ":irc.server 315 " << getNickname(client_fd) 
			<< " #chat :End of /WHO list\r\n";
	sendMessage(client_fd, endOfWho.str());

}

void Server::sendError(int client_fd, const std::string& errorCode, const std::string& command, const std::string& message)
{
	std::string nickname = getNickname(client_fd);
	if (nickname.empty())
		nickname = "*"; // Indiquer que le pseudo est inconnu

	std::ostringstream errorMsg;
	errorMsg << ":irc.server " << errorCode << " " << nickname << " " << command << " :" << message << "\r\n";
	send(client_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
}

void Server::sendMessage(int client_fd, const std::string& message)
{
	send(client_fd, message.c_str(), message.length(), 0);
}

bool Server::notregistered(int client_fd)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i]->getFd() == client_fd)
			return clients[i]->getNickname().empty() || clients[i]->getUsername().empty();
	}
	return true; // Si le client n'existe pas encore, il est considéré comme non enregistré.
}


Channel* Server::findChannel(const std::string& channelName)
{
	for (size_t i = 0; i < channels.size(); ++i)
	{
		if (channels[i]->getName() == channelName)
			return channels[i];
	}
	return NULL; // Aucun canal trouvé
}

// /*----Gestion de Nick ---*/

void Server::updateNicknameInChannels(const std::string& oldNick, const std::string& newNick, int fd)
{
    for (size_t i = 0; i < channels.size(); ++i)
    {
        if (channels[i]->isUserInChannel(fd))  // Vérification par FD
        {
            channels[i]->renameUser(oldNick, newNick, fd);
            // std::cout << " 🔄 Nickname mis à jour dans " << channels[i]->getName() << "\n";
        }
    }
}

void Channel::renameUser(const std::string& oldNick, const std::string& newNick, int fd)
{
    std::map<std::string, int>::iterator it = usersMap.find(oldNick);
    if (it != usersMap.end())
    {
        usersMap.erase(it);  // Supprimer l'ancien pseudo
        usersMap[newNick] = fd;  // Ajouter le nouveau pseudo
    }

    // 🛠 Mise à jour propre de la liste des utilisateurs (évite suppressions inutiles)
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (users[i] == fd)
        {
            users[i] = fd;  // Mise à jour en place
            break;
        }
    }

    // 🔄 Mise à jour des opérateurs si nécessaire
    for (size_t i = 0; i < operators.size(); ++i)
    {
        if (operators[i] == fd)  
        {
            operators[i] = fd;
            break;
        }
    }

    // 📌 Log des utilisateurs après mise à jour
    // std::cout << " 📌 usersMap après update: ";
    for (std::map<std::string, int>::iterator it = usersMap.begin(); it != usersMap.end(); ++it)
    {
        std::cout << "[" << it->first << " : " << it->second << "] ";
    }
    std::cout << std::endl;

    // std::cout << " 🔄 Mise à jour du pseudo dans le canal: " << oldNick << " → " << newNick << "\n";
}

bool Channel::isUserInChannel(int client_fd)
{
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (users[i] == client_fd)
        {
            return true;
        }
    }
    return false;
}



bool Server::isNicknameTaken(const std::string& nickname)
{
	std::vector<Client *>::iterator it;
	for (it = clients.begin(); it != clients.end(); ++it)
	{
		Client* client = *it;
		if (client->getNickname() == nickname)
			return true;
	}
	return false;
}

Client* Server::getClientByFd(int fd)
{
    // Parcours de la liste des clients pour trouver celui qui a le même file descriptor
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getFd() == fd)
        {
            return clients[i];  // Retourne le client avec le fd correspondant
        }
    }
    return NULL;  // Retourne nullptr si aucun client n'a ce fd
}
