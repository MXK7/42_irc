/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 15:21:03 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/11 13:58:03 by thlefebv         ###   ########.fr       */
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
        // 🔥 Envoi d'un message NOTICE pour forcer Irssi à garder l'affichage du channel
        std::ostringstream noticeMessage;
        noticeMessage << ":irc.server NOTICE " << getNickname(params.operator_fd) 
                    << " :You don't have permission to kick users from " << params.channelName << "\r\n";
        sendMessage(params.operator_fd, noticeMessage.str());
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
    kickMessage << ":" << getNickname(params.client_fd) << " KICK " << params.channelName << " " << params.nickname << "\r\n";
    channel->broadcast(kickMessage.str(), params.client_fd);

    if (channel->isOperator(params.operator_fd)) // 🔥 Vérifie que l'utilisateur a le droit de KICK
    {
        // Diffuser le message à tout le channel
        channel->broadcast(kickMessage.str(), params.client_fd);
        
        // Supprimer l'utilisateur du channel
        channel->removeUser(kicked_fd);
        send(kicked_fd, kickMessage.str().c_str(), kickMessage.str().size(), 0);// Informer l'utilisateur expulsé
        std::cout << "FD: " << params.client_fd << " kicked user " << params.nickname << " from channel " << params.channelName << std::endl;
    }
    else
    {
        // 🔥 Envoyer une erreur et un NOTICE pour éviter qu'Irssi "quitte" visuellement le channel
        sendError(params.operator_fd, "482", params.channelName, "You're not a channel operator");
        std::ostringstream noticeMessage;
        noticeMessage << ":irc.server NOTICE " << getNickname(params.operator_fd) 
                    << " :You don't have permission to kick users from " << params.channelName << "\r\n";
        sendMessage(params.operator_fd, noticeMessage.str());
    }

}
