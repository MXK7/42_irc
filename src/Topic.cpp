/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/03 10:55:37 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/13 15:03:41 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Channel.hpp"

void Server::handleTopic(const CommandParams& params)
{
    std::string nickname = getNickname(params.client_fd);
    Channel* channel = getChannelByName(params.channelName);
    if (!channel)
    {
        sendError(params.client_fd, "403", "TOPIC", "No such channel");
        return;
    }

    // ✅ 🔥 Vérifier si l'utilisateur veut juste voir le topic
    if (params.Arg.empty())
    {
        std::string currentTopic = channel->getTopic();
        if (currentTopic.empty())
        {
            sendMessage(params.client_fd, ":irc.server 331 " + params.channelName + " :No topic is set\r\n");
        }
        else
        {
            sendMessage(params.client_fd, ":irc.server 332 " + getNickname(params.client_fd) + " " 
                        + params.channelName + " :" + currentTopic + "\r\n");

            // 🔥 Ajouter aussi l'info sur **QUI** a changé le topic et quand (RFC 1459)
            std::ostringstream topicWhoTime;
            topicWhoTime << ":irc.server 333 " << getNickname(params.client_fd) << " " 
                         << params.channelName << " " << channel->getTopicSetter() << " " 
                         << channel->getTopicTimestamp() << "\r\n";
            sendMessage(params.client_fd, topicWhoTime.str());
        }
        return;
    }

    // ❌ Vérifier si le mode +t est activé et que l'utilisateur n'est pas opérateur
    if (channel->isTopicLock() && !channel->isOperator(params.client_fd))
    {
        sendError(params.client_fd, "482", "TOPIC", "You're not a channel operator");
        return;
    }

    // ✅ Modifier le topic si l'utilisateur est autorisé
    if(channel->isUserInChannel(nickname))
    {

    std::string newTopic = params.Arg[0];
    channel->setTopic(newTopic, getNickname(params.client_fd));
    channel->setTopic(params.Arg[0], getNickname(params.client_fd));
    channel->setTopicMetadata(getNickname(params.client_fd));                 // ⏳ Stocke l'horodatage

    // 🔥 Diffuser le changement de topic à tous les utilisateurs du canal
    std::ostringstream topicMsg;
    topicMsg << ":" << getNickname(params.client_fd) << " TOPIC " << params.channelName << " :" << newTopic << "\r\n";
    channel->broadcast(topicMsg.str());

    std::cout << "[DEBUG] " << getNickname(params.client_fd) << " changed the topic of " 
              << params.channelName << " to: " << newTopic << std::endl;
    }
}











