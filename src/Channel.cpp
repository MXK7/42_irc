/* ************************************************************************** */
/*                                                                            */
/*                                                        :::     ::::::::  */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmassoli <vmassoli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:30:02 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/11 18:14:10 by vmassoli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../includes/Client.hpp"
#include "../includes/Server.hpp"

Channel::Channel() {
	inviteOnly = false;
	isKey = false;
	isTopic = false;
	userLimit = -1; // ?? a verifier
	topic = "";
	key = "";
}

Channel::Channel(const std::string name) :
name(name) {}

// Channel::Channel(const std::string& channelName) : name(channelName) {}

std::string Channel::getName() const {
		return name;
	}

Channel::~Channel() {}

void Channel::addUser(int fd){
	users.push_back(fd);
}

void Channel::addOperator(int fd) {
	operators.push_back(fd);
}


void Channel::addInvitedUser(const std::string &nickname){
	invitedUsers.push_back(nickname);
}

bool Channel::isUserInvited(const std::string &nickname){
	return std::find(invitedUsers.begin(), invitedUsers.end(),
			nickname) != invitedUsers.end();
}

bool Channel::isUserInChannel(int fd) {
	return (std::find(users.begin(), users.end(), fd) != users.end());
}

bool Channel::isOperator(int fd) {
	return std::find(operators.begin(), operators.end(), fd) != operators.end();
}

bool Channel::isInviteOnly() const {
	return (inviteOnly);
}

void Channel::setInviteOnly(bool status){
	inviteOnly = status;
}



std::string Channel::getTopic() {
	return topic;
}

void Channel::setTopic(const std::string &newTopic) {
	topic = newTopic;
}

void Channel::broadcast(const std::string &message, int excludeFd) {
	for (size_t i = 0; i < users.size(); ++i) {
		if (static_cast<int>(users[i]) != excludeFd) {
			send(users[i], message.c_str(), message.size(), 0);
		}
	}
}
