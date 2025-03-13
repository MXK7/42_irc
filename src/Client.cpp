/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:24:16 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/26 13:59:48 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"


Client::~Client() {}

Client::Client(int fd, const std::string& name, const std::string& nickname, const std::string& hostname)
    : fd(fd), nickname(nickname), username(name), hostname(hostname) {}

std::string Client::getName() const {
	return name;
}

std::string Client::getUsername() const {
	return username;
}

std::string Client::getNickname() const {
	return nickname;
}

int Client::getFd() const {
	return fd;
}

void Client::setNickname(const std::string& nickname) {
	this->nickname = nickname;
}

void Client::setName(const std::string& name) {
	this->name = name;
}

void Client::setUsername(const std::string& username) {
	this->username = username;
}


int Server::getClientFdByNickname(const std::string& nickname)
{
    Client* client = getClientByNickname(nickname);
    if (client)
        return client->getFd();
    return -1; // Aucun client trouvé
}

bool Client::isAuthenticated() const { 
	return is_authenticated; 
}

void Client::authenticate() { 
	is_authenticated = true; 
}

bool Client::notregistered() const {
	 return !is_authenticated; 
}

std::string Client::getHostname() const {
    if (hostname.empty()) {
        // std::cout << " ❌ Hostname vide détecté pour " << nickname << " !" << std::endl;
        return "127.0.0.1";  // 🔥 Assigner un hostname par défaut si vide
    }
    return hostname;
}



void Client::setHostname(const std::string &host) {
	hostname = host;
}

void Server::handleWho(const CommandParams& params)
{
    std::string nickname = params.nickname;
    Client* client = getClientByNickname(nickname);

    if (!client) {
        sendError(params.client_fd, "401", "WHO", "No such nick"); // 401: ERR_NOSUCHNICK
        return;
    }

    // Construire la réponse WHO
    std::ostringstream whoReply;
    whoReply << ":irc.server 352 " << getNickname(params.client_fd) << " * " 
             << client->getUsername() << " " << client->getHostname() << " "
             << client->getNickname() << " H :0 " << client->getName() << "\r\n";

    sendMessage(params.client_fd, whoReply.str());

    // Envoyer la fin de la liste
    std::ostringstream endWhoReply;
    endWhoReply << ":irc.server 315 " << getNickname(params.client_fd)
                << " " << nickname << " :End of /WHO list\r\n";
    sendMessage(params.client_fd, endWhoReply.str());
}
