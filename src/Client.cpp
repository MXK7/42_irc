/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmassoli <vmassoli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:24:16 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/11 20:19:29 by vmassoli         ###   ########.fr       */
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
