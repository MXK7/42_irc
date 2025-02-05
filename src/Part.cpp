/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:43:50 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/05 15:45:48 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::handlePart(const CommandParams& params)
{
    Channel* channel = getChannelByName(params.channelName);
    if (!channel)
    {
        sendError(params.client_fd, "403", "PART", "No such channel");
        return;
    }

    std::string nickname = getNickname(params.client_fd);

    if (!channel->isUserInChannel(nickname))
    {
        sendError(params.client_fd, "442", "PART", "You're not in that channel");
        return;
    }

    // 🔥 Construire le message PART conforme aux standards IRC
    std::ostringstream partMessage;
    partMessage << ":" << nickname << "!" << nickname << "@irc.server PART " << params.channelName;
    if (!params.Arg.empty())
    {
        partMessage << " :" << params.Arg[0];  // Ajoute la raison du départ
    }
    partMessage << "\r\n";

    // 🔥 Envoyer le message PART au client lui-même
    sendMessage(params.client_fd, partMessage.str());

    // 🔥 Informer les autres membres du canal que l'utilisateur est parti
    channel->broadcast(partMessage.str(), params.client_fd);

    // 🔥 Supprimer l'utilisateur du canal
    channel->removeUser(params.client_fd);

    // 🔥 Si le canal est vide après le départ, on le supprime
    if (channel->getUserCount() == 0)
    {
        std::cout << "[DEBUG] 🔥 Suppression du canal vide : " << params.channelName << std::endl;
        channels.erase(std::remove(channels.begin(), channels.end(), channel), channels.end());
        delete channel;
    }
}


