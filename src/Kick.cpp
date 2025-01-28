/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 15:21:03 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/28 15:21:04 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"


void Server::handleKick(const CommandParams &params)
{
    Channel *channel = NULL;
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
        sendError(params.operator_fd, "403", params.channelName, "No such channel");
        return;
    }

    if (!channel->isOperator(params.operator_fd))
    {
        sendError(params.operator_fd, "481", params.channelName, "You're not a channel operator");
        return;
    }

    if (!channel->isUserInChannel(params.nickname))
    {
        sendError(params.operator_fd, "441", params.nickname, "They aren't on that channel");
        return;
    }

    int kicked_fd = channel->getUserFdByNickname(params.nickname);

    // Diffuser le message et supprimer l'utilisateur
    std::ostringstream kickMessage;
    kickMessage << ":" << getNickname(params.client_fd) << " KICK "
                << params.channelName << " " << params.nickname << "\r\n";
    channel->broadcast(kickMessage.str(), params.client_fd);

    channel->removeUser(kicked_fd);

    // Informer l'utilisateur expulsé
    send(kicked_fd, kickMessage.str().c_str(), kickMessage.str().size(), 0);

    std::cout << "FD: " << params.client_fd << " kicked user " << params.nickname
              << " from channel " << params.channelName << std::endl;
}
