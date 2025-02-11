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


std::string Channel::getName() const {
		return name;
	}

Channel::~Channel() {}

void Channel::addUser(int fd,  const std::string &nickname){
	if (std::find(users.begin(), users.end(), fd) == users.end()) {
		users.push_back(fd);
	}

	if (usersMap.find(nickname) == usersMap.end()) {
		usersMap[nickname] = fd;
	}
}

/*-----------------------------------------------------------------------------------------*/
void Channel::addOperator(int fd) {
	operators.push_back(fd);
}

void Channel::removeOperator(int fd)
{
    operators.erase(std::remove(operators.begin(), operators.end(), fd), operators.end());
}

bool Channel::isOperator(int fd) {
	return std::find(operators.begin(), operators.end(), fd) != operators.end();
}

/*----------------------------------------------------------------------------------*/

void Channel::addInvitedUser(const std::string &nickname){
	invitedUsers.push_back(nickname);
}

bool Channel::isUserInvited(const std::string& nickname)
{
    bool invited = (std::find(invitedUsers.begin(), invitedUsers.end(), nickname) != invitedUsers.end());
    std::cout << "[DEBUG] Checking if " << nickname << " is invited: " << (invited ? "YES" : "NO") << std::endl;
    return invited;
}


bool Channel::isUserInChannel(const std::string &nickname) {
	return (usersMap.find(nickname) != usersMap.end());
}


bool Channel::isTopicLock() const {
		return isTopic;
	}

void Channel::setTopicLock(bool status){
	isTopic = status;
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

int Channel::getUserFdByNickname(const std::string &nickname)
{
	std::map<std::string, int>::iterator it = usersMap.find(nickname);
	if (it != usersMap.end()) {
		return it->second;
	}
	return -1;
}

void Channel::setTopic(const std::string &newTopic) {
	topic = newTopic;
}

void Channel::broadcast(const std::string &message, int excludeFd) 
{
    for (size_t i = 0; i < users.size(); ++i) 
    {
        if (users[i] != excludeFd)  // Ne pas envoyer le message à celui qui l'a envoyé
        {
            send(users[i], message.c_str(), message.size(), 0);
            std::cout << "[DEBUG] Message envoyé à FD " << users[i] << ": " << message << std::endl;
        }
    }
}

void Channel::removeUser(int fd) {

	std::vector<int>::iterator it = std::find(users.begin(), users.end(), fd);
	if (it != users.end()) {
		users.erase(it);
		std::cout << "FD " << fd << " erased" << std::endl;
	} else {
		std::cout << "FD " << fd << " not found" << std::endl;
	}

	for (std::map<std::string, int>::iterator mapIt = usersMap.begin();
											mapIt != usersMap.end(); ++mapIt) {
		if (mapIt->second == fd) {
			usersMap.erase(mapIt);
			break;
		}
	}
}

/*-------------------------------------------------------------------------------*/
void Channel::setKey(const std::string &newKey)
{
    key = newKey;
    isKey = true;  // 🔥 S'assurer que le flag isKey est activé !
    std::cout << "[DEBUG] 🔑 Clé définie pour " << name << " : " << key << std::endl;
}

void Channel::clearKey()
{
    key.clear();
    isKey = false;
    std::cout << "[DEBUG] 🔓 Clé supprimée pour " << name << std::endl;
}

bool Channel::hasKey() const
{
    return isKey;
}

std::string Channel::getKey() const
{
	return key;
}
/*-------------------------------------------------------------------------*/


int Channel::getUserLimit() const {
	return userLimit;
}

void Channel::setUserLimit(int limit) {
	userLimit = limit;
}

void Channel::clearUserLimit() {
	userLimit = -1;
}

