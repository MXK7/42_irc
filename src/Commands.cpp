/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mehdi <mehdi@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:55:54 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/13 11:04:29 by mehdi            ###   ########.fr       */
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
