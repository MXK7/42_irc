/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:24:16 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/17 15:02:40 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Channel.hpp"
#include "../includes/Server.hpp"

// Client::Client() {}

Client::~Client() {}


Client::Client(int fd, const std::string& name, const std::string& nickname)
	: fd(fd), name(name), nickname(nickname) {}

std::string Client::getName() const {
	return name;
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


