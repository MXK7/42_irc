/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:33:51 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/30 10:47:39 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void Server::handleJoin(const CommandParams& params)
{
	Channel* channel = NULL;
	std::string nickname = getNickname(params.client_fd);

	std::cout << "[DEBUG] Client FD " << params.client_fd << " (" << nickname << ") trying to join channel " << params.channelName << std::endl;

	for (size_t i = 0; i < channels.size(); ++i) 
	{
		if (channels[i]->getName() == params.channelName) 
		{
			channel = channels[i];
			break;
		}
	}

	if (!channel) 
	{
		try 
		{
			channel = new Channel(params.channelName);
			channels.push_back(channel);
			channel->addOperator(params.client_fd);

			std::cout << "[DEBUG] New channel created: " << params.channelName << " by FD " << params.client_fd << " (" << nickname << ")" << std::endl;

			std::ostringstream joinMessage;
			joinMessage << ":irc.server 332 " << params.client_fd << " " << params.channelName << " :Channel created\n";
			sendMessage(params.client_fd, joinMessage.str());
		}
		catch (const std::bad_alloc&) 
		{
			std::cerr << "[ERROR] Failed to allocate memory for channel: " << params.channelName << std::endl;
			sendError(params.client_fd, "500", "JOIN", "Internal server error");
			return;
		}
	}

	if (channel->isInviteOnly() && !channel->isUserInvited(nickname)) // Vérifier si le canal est en mode invite-only et si l'utilisateur est invité
	{
		std::cout << "[DEBUG] FD " << params.client_fd << " (" << nickname << ") denied access to invite-only channel " << params.channelName << std::endl;
		sendError(params.client_fd, "473", "JOIN", "Cannot join channel (Invite only)");
		return;
	}

	if (!channel->isUserInChannel(nickname))// Vérifier si l'utilisateur est déjà dans le canal
	{
		channel->addUser(params.client_fd, nickname);
		std::cout << "[DEBUG] FD " << params.client_fd << " (" << nickname << ") successfully joined channel " << params.channelName << std::endl;

		std::ostringstream joinMessage;
		joinMessage << ":irc.server 331 " << params.client_fd << " " << params.channelName << " :You have joined the channel\n";
		sendMessage(params.client_fd, joinMessage.str());
		channel->broadcast(joinMessage.str(), params.client_fd);
	}
	else
	{
		std::cout << "[DEBUG] FD " << params.client_fd << " (" << nickname << ") is already in " 
				  << params.channelName << std::endl;

		sendError(params.client_fd, "443", "JOIN", "You're already in the channel");
	}
}



void Channel::listUsers()
{
	std::cout << "[DEBUG] Users in " << name << ": ";
	for (std::map<std::string, int>::iterator it = usersMap.begin(); it != usersMap.end(); ++it)
	{
		std::cout << it->first << " (FD " << it->second << "), ";
	}
	std::cout << std::endl;
}
