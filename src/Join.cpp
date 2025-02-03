/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:33:51 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/03 17:27:18 by thlefebv         ###   ########.fr       */
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

            sendMessage(params.client_fd, ":irc.server 332 " + params.channelName + " :Channel created\r\n");
        }
        catch (const std::bad_alloc&) 
        {
            sendError(params.client_fd, "500", "JOIN", "Internal server error");
            return;
        }
    }

    if (channel->isInviteOnly() && !channel->isUserInvited(nickname))
    {
        sendError(params.client_fd, "473", "JOIN", "Cannot join channel (Invite only)");
        return;
    }

    if (!channel->isUserInChannel(nickname))
    {
        channel->addUser(params.client_fd, nickname);

        std::ostringstream joinAnnounce;
        joinAnnounce << ":" << nickname << " JOIN " << params.channelName << "\r\n";
        channel->broadcast(joinAnnounce.str(), params.client_fd);

        std::cout << "[DEBUG] FD " << params.client_fd << " (" << nickname << ") successfully joined channel " << params.channelName << std::endl;

        std::ostringstream namesMessage;
        namesMessage << ":irc.server 353 " << nickname << " = " << params.channelName << " :";
        namesMessage << channel->listUsers();
        namesMessage << "\r\n";
        sendMessage(params.client_fd, namesMessage.str());

        std::ostringstream endOfNames;
        endOfNames << ":irc.server 366 " << nickname << " " << params.channelName << " :End of /NAMES list\r\n";
        sendMessage(params.client_fd, endOfNames.str());

        // 🔥 Mise à jour du nombre total d'utilisateurs dans le canal
        std::ostringstream totalUsersMessage;
        totalUsersMessage << ":irc.server 322 " << nickname << " " << params.channelName
                          << " " << channel->getUserCount() << " :Users in channel\r\n";
        channel->broadcast(totalUsersMessage.str());
    }
    else
        sendError(params.client_fd, "443", "JOIN", "You're already in the channel");
}


std::string Channel::listUsers() const
{
    std::ostringstream oss;
    for (std::map<std::string, int>::const_iterator it = usersMap.begin(); it != usersMap.end(); ++it)
    {
        oss << it->first << " "; // Affiche le nom de chaque utilisateur suivi d'un espace
    }
    return oss.str(); // Retourne la liste des utilisateurs sous forme de chaîne
}

int Channel::getUserCount() const
{
    return usersMap.size();
}
