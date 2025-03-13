/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:13:04 by vmassoli          #+#    #+#             */
/*   Updated: 2025/02/18 13:45:30 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// #include "Server.hpp"
// #include "Client.hpp"

#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <stdexcept>

class Channel
{
private:

	std::map<std::string, int> usersMap;
	std::vector<int> users;
	std::vector<int> operators;
	std::vector<std::string> invitedUsers;
	bool inviteOnly;		// (+i)
	bool isKey;				//(+k)
	bool isTopic;			//(+t)
	int userLimit;			// (+l)
	std::string name;
	std::string topic;      //(+t)
	std::string topicSetter;  // 🔥 Stocke le pseudo du dernier utilisateur ayant changé le topic
    time_t topicTimestamp;	
	std::string key;		// (+k)

public:

	Channel();
	Channel(const std::string name);
	~Channel();


	void addUser(int fd,  const std::string &nickname);
	void addOperator(int fd);
	void addInvitedUser(const std::string &nickname);
	bool isUserInChannel(const std::string &nickname);
	bool isOperator(int fd);
	bool isTopicLock() const;
	void setTopicLock(bool status);
	void setTopic(const std::string &newTopic, const std::string &setter);
	std::string getTopicSetter() const;
    time_t getTopicTimestamp() const;
	void setTopicMetadata(const std::string& setter);
	std::pair<std::string, time_t> getTopicMetadata() const;


	bool isUserInvited(const std::string &nickname);
	bool isInviteOnly() const;
	void setInviteOnly(bool status);
	std::string getTopic();
	std::string getName() const;
	int getUserFdByNickname(const std::string &nickname);
	void broadcast(const std::string &message, int excludeFd = -1);
	void removeUser(int fd);

	void setKey(const std::string &key);
	void clearKey();
	void setUserLimit(int limit);
	bool hasUserLimit() const;
	void clearUserLimit();
	bool hasKey() const;
	int getUserLimit() const;
	std::string getKey() const;
	void removeOperator(int fd);
	std::string listUsers() const;
	int getUserCount() const;
	void renameUser(const std::string& oldNick, const std::string& newNick, int fd);
	bool isUserInChannel(int client_fd);

};
