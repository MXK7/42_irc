/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmassoli <vmassoli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 17:13:04 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/12 16:44:22 by vmassoli         ###   ########.fr       */
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
	bool isKey;
	bool isTopic;
	int userLimit;			// (+l)
	std::string name;
	std::string topic;
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
	bool isUserInvited(const std::string &nickname);
	bool isInviteOnly() const;
	void setInviteOnly(bool status);
	std::string getTopic();
	std::string getName() const;
	int getUserFdByNickname(const std::string &nickname);
	void setTopic(const std::string &newTopic);
	void broadcast(const std::string &message, int excludeFd = -1);
	void removeUser(int fd);

};
