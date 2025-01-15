/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmassoli <vmassoli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:55:54 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/15 14:50:08 by vmassoli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
// #include "../includes/Client.hpp"
// #include "../includes/Channel.hpp"

std::string Server::getName(int client_fd)
{
	for (size_t i = 0; i < clients.size(); ++i) {
		Client* client = clients[i];
		if (client->getFd() == client_fd) {
			return client->getName();
		}
	}
	return ""; // utiliser une exception avec thrwo ????
}

std::string Server::getNickname(int client_fd)
{
	for (size_t i = 0; i < clients.size(); ++i) {
		Client* client = clients[i];
		if (client->getFd() == client_fd) {
			return client->getNickname();
		}
	}
	return "";
}

Channel* Server::getChannelByName(const std::string &name) {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == name) {
			return channels[i];
		}
	}
	return NULL;
}


/*____________________________________________________________________*/

void Server::handleJoin(const CommandParams &params)
{
	// ?? on affiche le prompt : join fd =  , channel =   , nickname =     )
	Channel *channel = NULL;
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == params.channelName) {
			channel = channels[i];
			break;
		}
	}

	if (!channel) {
		try {
				channel = new Channel(params.channelName);
				channels.push_back(channel);

				channel->addOperator(params.client_fd);
				std::cout << "New channel : " << params.channelName << std::endl;
			} catch (const std::bad_alloc&) {
				std::cerr << "ERROR" << std::endl;
				return;
			}
		}

	if (channel->isInviteOnly() && channel->isUserInvited(params.nickname)) {
		std::cerr << "Mode invite-only " << params.client_fd <<
					": acces denied"<< std::endl;
		std::string errorMsg = "Channel is invite-only.\n";
		send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}
	if (!channel->isUserInChannel(params.nickname)) {

		channel->addUser(params.client_fd, params.nickname);
		std::cout << "FD: " << params.client_fd << ") joined the channel "
				<< params.channelName << std::endl;

		std::string joinMessage = ":" + getNickname(params.client_fd) + " JOIN " +
			 params.channelName + "\n";
		send(params.client_fd, joinMessage.c_str(), joinMessage.size(), 0);

		channel->broadcast(joinMessage, params.client_fd);
	}
}

/*___________________________________________________________*/

void Server::handleInvit(const CommandParams &params)
{
	Channel *channel = NULL;

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == params.channelName) {
			channel = channels[i];
			break;
		}
	}
		// ?? gestion mess d erreur
	if (!channel) {
		std::cerr << "Error: Channel " << params.channelName
					<< " does not exist!" << std::endl;
		return;
	}

	if (!channel->isOperator(params.operator_fd)) {
		std::string errorMsg = "ERROR : You are not an operator in channel "
							+ params.channelName + ".\n";
		send(params.operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	Client *targetClient = NULL;
	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i]->getNickname() == params.nickname) {
			targetClient = clients[i];
			break;
		}
	}

	if (!targetClient) {
		std::string errorMsg = "ERROR : User " +
							params.nickname + " not found.\n";
		send(params.operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	channel->addInvitedUser(params.nickname);

	std::string inviteMsg = ":" + getNickname(params.operator_fd) +
						" INVITE " + params.nickname + " " + params.channelName + "\n";
	send(targetClient->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);

	std::string confirmationMsg = "You have invited " + params.nickname +
							" to channel " + params.channelName + ".\n";
	send(params.operator_fd, confirmationMsg.c_str(), confirmationMsg.size(), 0);

}

/*___________________________________________________________*/

void Server::handleKick(const CommandParams &params)
{
	Channel *channel = NULL;
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == params.channelName) {
			channel = channels[i];
			break;
		}
	}

	if (!channel) {
		std::cerr << "Error: Channel " << params.channelName
					<< " does not exist!" << std::endl;
		return;
	}

	 if (!channel->isOperator(params.operator_fd)) {
		std::cerr << "Error: You are not an operator in this channel!" << std::endl;
		std::string errorMsg = "You are not an operator in this channel.\n";
		send(params.operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	if (!channel->isUserInChannel(params.nickname)) {
		std::cerr << "Error: User " << params.nickname
					 << " is not in the channel!" << std::endl;
		std::string errorMsg = "User " + params.nickname + " is not in the channel.\n";
		send(params.operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	int kicked_fd = channel->getUserFdByNickname(params.nickname);

	std::string kickMessage = ":" + getNickname(params.client_fd) + " KICK "
						+ params.channelName + " " + params.nickname + "\n";
	channel->broadcast(kickMessage, params.client_fd);

	channel->removeUser(kicked_fd);

	std::string kickedMessage = "You have been kicked from " + params.channelName
						+ " by " + getNickname(params.client_fd) + "\n";
	send(kicked_fd, kickedMessage.c_str(), kickedMessage.size(), 0);

	std::cout << "FD: " << params.client_fd << " kicked user " << params.nickname
						<< " from channel " << params.channelName << std::endl;
}


void Server::handleTopic(int client_fd, const CommandParams &params) {
	if (params.additionalParams.empty()) {
		std::string errorMsg = "Error: No channel specified.\n";
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	const std::string &channelName = params.additionalParams[0];
	Channel *channel = getChannelByName(channelName); // Implémenter getChannelByName si ce n'est pas fait

	if (!channel) {
		std::string errorMsg = "Error: Channel does not exist.\n";
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	if (params.additionalParams.size() == 1) {
		const std::string &currentTopic = channel->getTopic();

		if (currentTopic.empty()) {
			std::string msg = "No topic is set for " + channelName + ".\n";
			send(client_fd, msg.c_str(), msg.size(), 0);
		} else {
			std::string msg = "Current topic for " + channelName + ": " + currentTopic + "\n";
			send(client_fd, msg.c_str(), msg.size(), 0);
		}
	} else {

		const std::string &newTopic = params.additionalParams[1];


		if (channel->isTopicLock() && !channel->isOperator(client_fd)) {
			std::string errorMsg = "Error: Only channel operators can change the topic.\n";
			send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
			return;
		}


		channel->setTopic(newTopic);


		std::string broadcastMsg = "Topic for " + channelName + " set to: " + newTopic + "\n";
		channel->broadcast(broadcastMsg);


		std::string confirmMsg = "Topic updated successfully.\n";
		send(client_fd, confirmMsg.c_str(), confirmMsg.size(), 0);
	}
}

void Server::handleMode(const CommandParams &params)
{
	const std::string &channelName = params.channelName;
	const std::string &modes = params.additionalParams[0];
	std::vector<std::string> modeParams(params.additionalParams.begin() + 1,
							params.additionalParams.end());

	Channel *channel = NULL;

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == channelName) {
			channel = channels[i];
			break;
		}
	}

	if (!channel) {
		std::string errorMsg = "Error: Channel " + channelName + " does not exist.\n";
		send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	bool addingMode = true;
	size_t paramIndex = 0;

	for (size_t i = 0; i < modes.size(); ++i) {
		char mode = modes[i];
		if (mode == '+') {
			addingMode = true;
			continue;
		} else if (mode == '-') {
			addingMode = false;
			continue;
		}

		switch (mode) {
		case 'i': // Invite-only
			channel->setInviteOnly(addingMode);
			break;
		case 't': // Topic lock
			channel->setTopicLock(addingMode);
			break;
		case 'k': // Channel key
			if (addingMode) {
				if (paramIndex >= modeParams.size()) {
					std::string errorMsg = "Error: Missing parameter for +k mode.\n";
					send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
					return;
				}
				channel->setKey(modeParams[paramIndex++]);
			} else {
				channel->clearKey();
			}
			break;
		case 'l': // User limit
			if (addingMode) {
				if (paramIndex >= modeParams.size()) {
					std::string errorMsg = "Error: Missing parameter for +l mode.\n";
					send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
					return;
				}
				int limit = std::atoi(modeParams[paramIndex++].c_str());
				channel->setUserLimit(limit);
			} else {
				channel->clearUserLimit();
			}
			break;
		default:
			std::string errorMsg = "Error: Unknown mode " + std::string(1, mode) + ".\n";
			send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
			return;
		}
	}

	std::string successMsg = "Mode " + modes + " applied to " + channelName + ".\n";
	send(params.client_fd, successMsg.c_str(), successMsg.size(), 0);
}
