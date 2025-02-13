/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 15:21:03 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/13 19:16:55 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::handleKick(const CommandParams &params)
{
    Channel *channel = NULL;

    // 🔎 Vérifier si le canal existe
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

    // 🔎 Vérifier si l'utilisateur est dans le canal
    if (!channel->isUserInChannel(getNickname(params.operator_fd))) 
    {
        sendError(params.operator_fd, "441", getNickname(params.operator_fd), "You're not on that channel");
        return;
    }

    // 🔎 Vérifier si l'utilisateur est opérateur dans le canal
    if (!channel->isOperator(params.operator_fd))
    {
        // ✅ Envoyer le message DANS LE CHANNEL au lieu d'un NOTICE global
        std::ostringstream kickErrorMessage;
        kickErrorMessage << ":irc.server PRIVMSG " << params.channelName 
                         << " :" << getNickname(params.operator_fd) 
                         << " tried to kick a user but is not an operator.\r\n";
        
        channel->broadcast(kickErrorMessage.str());
        return;
    }

    // 🔎 Vérifier si l'utilisateur à expulser est dans le canal
    if (!channel->isUserInChannel(params.nickname))
    {
        sendError(params.operator_fd, "441", params.nickname, "They aren't on that channel");
        return;
    }

    int kicked_fd = channel->getUserFdByNickname(params.nickname);

    // ❌ Supprimer l'utilisateur du canal
    channel->removeUser(kicked_fd);

    // 🔥 Diffuser le message de kick à tous les membres du canal
    std::ostringstream kickMessage;
    kickMessage << ":" << getNickname(params.operator_fd) << " KICK " 
                << params.channelName << " " << params.nickname << "\r\n";
    channel->broadcast(kickMessage.str());

    // 📩 Informer l'utilisateur expulsé
    send(kicked_fd, kickMessage.str().c_str(), kickMessage.str().size(), 0);

    std::cout << "FD: " << params.operator_fd << " kicked user " 
              << params.nickname << " from channel " << params.channelName << std::endl;
}





void Server::removeChannel(const std::string &channelName)
{
    for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if ((*it)->getName() == channelName)
        {
            delete *it; // 🔥 Libère la mémoire
            channels.erase(it);
            std::cout << "[DEBUG] 🔥 Suppression du canal vide : " << channelName << std::endl;
            return;
        }
    }
}

void Server::sendMessageToChannel(const std::string& channelName, const std::string& message)
{
    Channel* channel = getChannelByName(channelName);
    if (!channel) return;

    std::ostringstream privMsg;
    privMsg << ":irc.server PRIVMSG " << channelName << " :" << message << "\r\n";

    channel->broadcast(privMsg.str());
}



