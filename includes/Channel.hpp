/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:13:04 by vmassoli          #+#    #+#             */
/*   Updated: 2025/02/04 16:24:04 by thlefebv         ###   ########.fr       */
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
	std::string topic;		//(+t)
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
	bool isUserInvited(const std::string &nickname);
	bool isInviteOnly() const;
	void setInviteOnly(bool status);
	std::string getTopic();
	std::string getName() const;
	int getUserFdByNickname(const std::string &nickname);
	void setTopic(const std::string &newTopic);
	void broadcast(const std::string &message, int excludeFd = -1);
	void removeUser(int fd);

	void setKey(const std::string &key);
	void clearKey();
	void setUserLimit(int limit);
	void clearUserLimit();
	bool hasKey() const;
	int getUserLimit() const;
	std::string getKey() const;
	void removeOperator(int fd);
	std::string listUsers() const;
	int getUserCount() const;

};
