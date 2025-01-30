/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:24:16 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/30 13:32:59 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"


Client::~Client() {}


 Client::Client(int fd, const std::string& name, const std::string& nickname)
        : fd(fd), name(name), nickname(nickname), username(""), is_authenticated(false) {}

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
