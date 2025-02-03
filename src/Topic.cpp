/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 10:55:37 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/03 16:43:40 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"

void Server::handleTopic(const CommandParams& params)
{
    Channel* channel = getChannelByName(params.channelName);
    if (!channel)
    {
        sendError(params.client_fd, "403", "TOPIC", "No such channel");
        return;
    }

    // 🔥 Vérification si l'utilisateur veut juste voir le topic
    if (params.Arg.empty())
    {
        std::string currentTopic = channel->getTopic();
        if (currentTopic.empty())
            sendMessage(params.client_fd, ":irc.server 331 " + params.channelName + " :No topic is set\r\n");
        else
            sendMessage(params.client_fd, ":irc.server 332 " + params.channelName + " :" + currentTopic + "\r\n");
        return;
    }

    // 🔥 Vérification si le mode +t est activé
    if (channel->isTopicLock() && !channel->isOperator(params.client_fd))
    {
        sendError(params.client_fd, "482", "TOPIC", "You're not a channel operator");
        return;
    }

    // ✅ Modifier le topic si l'utilisateur est autorisé
    channel->setTopic(params.Arg[0]);

    // 🔥 Diffuser le changement de topic à tous les utilisateurs du canal
    std::ostringstream topicMsg;
    topicMsg << ":" << getNickname(params.client_fd) << " TOPIC " << params.channelName << " :" << params.Arg[0] << "\r\n";
    channel->broadcast(topicMsg.str());

    std::cout << "[DEBUG] " << getNickname(params.client_fd) << " changed the topic of " 
              << params.channelName << " to: " << params.Arg[0] << std::endl;
}










