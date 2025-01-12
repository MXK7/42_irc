/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmassoli <vmassoli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:55:54 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/12 12:34:50 by vmassoli         ###   ########.fr       */
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
	return ""; // utiliser une exception ????
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

void Server::handleJoin(int client_fd, const std::string &channelName, const std::string &nickname)
{
	Channel *channel = NULL;
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == channelName) {
			channel = channels[i];
			break;
		}
	}

	if (!channel) {
		try {
				channel = new Channel(channelName);
				channels.push_back(channel);

				channel->addOperator(client_fd);
				std::cout << "New channel : " << channelName << std::endl;
			} catch (const std::bad_alloc&) {
				std::cerr << "ERROR" << std::endl;
				return;
			}
		}

	if (channel->isInviteOnly() && channel->isUserInvited(nickname)) {
		std::cerr << "Mode invite-only " << client_fd <<
					": acces denied"<< std::endl;
		std::string errorMsg = "Channel is invite-only.\n";
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}
	if (!channel->isUserInChannel(client_fd)) {

		channel->addUser(client_fd);
		std::cout << "FD: " << client_fd << ") joined the channel "
				<< channelName << std::endl;

		std::string joinMessage = ":" + getNickname(client_fd) + " JOIN " +
			 channelName + "\n";
		send(client_fd, joinMessage.c_str(), joinMessage.size(), 0);

		channel->broadcast(joinMessage, client_fd);
	}
}

/*___________________________________________________________*/

void Server::handleInvit(int operator_fd, const std::string &channelName,
				const std::string &nickname)
{
	Channel *channel = NULL;

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == channelName) {
			channel = channels[i];
			break;
		}
	}
		// ?? gestion mess d erreur
	if (!channel) {
		std::string errorMsg = "ERROR : Channel " + channelName +
							" does not exist.\n";
		send(operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	if (!channel->isOperator(operator_fd)) {
		std::string errorMsg = "ERROR : You are not an operator in channel "
							+ channelName + ".\n";
		send(operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	Client *targetClient = NULL;
	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i]->getNickname() == nickname) {
			targetClient = clients[i];
			break;
		}
	}

	if (!targetClient) {
		std::string errorMsg = "ERROR : User " +
							nickname + " not found.\n";
		send(operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	channel->addInvitedUser(nickname);

	std::string inviteMsg = ":" + getNickname(operator_fd) +
						" INVITE " + nickname + " " + channelName + "\n";
	send(targetClient->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);

	std::string confirmationMsg = "You have invited " + nickname +
							" to channel " + channelName + ".\n";
	send(operator_fd, confirmationMsg.c_str(), confirmationMsg.size(), 0);

}
