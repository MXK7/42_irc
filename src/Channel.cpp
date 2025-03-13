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
    // std::cout << " Checking if " << nickname << " is invited: " << (invited ? "YES" : "NO") << std::endl;
    return invited;
}


bool Channel::isUserInChannel(const std::string &nickname) {
	return (usersMap.find(nickname) != usersMap.end());
}


bool Channel::isInviteOnly() const {
	return (inviteOnly);
}

void Channel::setInviteOnly(bool status){
	inviteOnly = status;
}


int Channel::getUserFdByNickname(const std::string &nickname)
{
	std::map<std::string, int>::iterator it = usersMap.find(nickname);
	if (it != usersMap.end()) {
		return it->second;
	}
	return -1;
}


/*-------------------------------------------------------------- */

std::string Channel::getTopic() {
	return topic;
}

void Channel::setTopic(const std::string &newTopic, const std::string &setter)
{
    topic = newTopic;
    topicSetter = setter;
    topicTimestamp = time(NULL);  // ⏳ Stocker l'horodatage actuel
}


bool Channel::isTopicLock() const {
		return isTopic;
	}

void Channel::setTopicLock(bool status){
	isTopic = status;
}

std::string Channel::getTopicSetter() const
{
	return topicSetter;
}

time_t Channel::getTopicTimestamp() const
{
	return topicTimestamp;
}

// Définir l'auteur et l'horodatage du topic
void Channel::setTopicMetadata(const std::string& setter)
{
    topicSetter = setter;
    topicTimestamp = time(NULL);
}

// Récupérer l'auteur et l'horodatage du topic
std::pair<std::string, time_t> Channel::getTopicMetadata() const
{
    return std::make_pair(topicSetter, topicTimestamp);
}


/*----------------------------------------------------------------*/



void Channel::broadcast(const std::string &message, int excludeFd) 
{
	// std::cout << " Broadcasting message in channel: " << message;
    for (size_t i = 0; i < users.size(); ++i) 
    {
        if (users[i] != excludeFd)  // Ne pas envoyer le message à celui qui l'a envoyé
        {
            int client_fd = users[i];
            if (send(client_fd, message.c_str(), message.size(), 0) == -1)
            {
                std::cerr << "[ERROR] Failed to send message to FD " << client_fd << std::endl;
            }
            else
            {
                // std::cout << " Message envoyé à FD " << client_fd << ": " << message << std::endl;
            }
        }
    }
}

void Channel::removeUser(int fd)
{
    std::vector<int>::iterator it = std::find(users.begin(), users.end(), fd);
    if (it != users.end()) {
        users.erase(it);
        // std::cout << " FD " << fd << " removed from users list." << std::endl;
    } else {
        // std::cout << " FD " << fd << " not found in users list." << std::endl;
    }

    for (std::map<std::string, int>::iterator mapIt = usersMap.begin(); mapIt != usersMap.end(); ) {
        if (mapIt->second == fd) {
            // std::cout << " Removing user " << mapIt->first << " from usersMap." << std::endl;
            usersMap.erase(mapIt++); // ✅ Supprime correctement en avançant l'itérateur
        }
        else {
            ++mapIt;
        }
    }

    // ✅ Ajout des logs après suppression
    // std::cout << " 📌 Liste users après KICK : ";
    for (size_t i = 0; i < users.size(); ++i)
    {
        std::cout << users[i] << " ";
    }
    std::cout << std::endl;

    // std::cout << " 📌 usersMap après KICK : ";
    for (std::map<std::string, int>::iterator it = usersMap.begin(); it != usersMap.end(); ++it)
    {
        std::cout << "[" << it->first << " : " << it->second << "] ";
    }
    std::cout << std::endl;
}


/*-------------------------------------------------------------------------------*/

void Channel::setKey(const std::string &newKey)
{
    key = newKey;
    isKey = true;  // 🔥 S'assurer que le flag isKey est activé !
    // std::cout << " 🔑 Clé définie pour " << name << " : " << key << std::endl;
}

void Channel::clearKey()
{
    key.clear();
    isKey = false;
    // std::cout << " 🔓 Clé supprimée pour " << name << std::endl;
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

bool Channel::hasUserLimit() const
{
    return userLimit > 0;
}
/*--------------------------------------------------------------------- */