/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:33:51 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/29 10:28:04 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"


void Server::handleJoin(const CommandParams& params)
{
    Channel* channel = NULL;
    std::string nickname = getNickname(params.client_fd);

    std::cout << "[DEBUG] Client FD " << params.client_fd << " (" << nickname 
              << ") trying to join channel " << params.channelName << std::endl;

    // Vérifier si le canal existe déjà
    for (size_t i = 0; i < channels.size(); ++i) 
    {
        if (channels[i]->getName() == params.channelName) 
        {
            channel = channels[i];
            break;
        }
    }

    // Si le canal n'existe pas, le créer
    if (!channel) 
    {
        try 
        {
            channel = new Channel(params.channelName);
            channels.push_back(channel);
            channel->addOperator(params.client_fd);

            std::cout << "[DEBUG] New channel created: " << params.channelName << " by FD " 
                      << params.client_fd << " (" << nickname << ")" << std::endl;

            std::ostringstream joinMessage;
            joinMessage << ":irc.server 332 " << params.client_fd << " " << params.channelName << " :Channel created\n";
            send(params.client_fd, joinMessage.str().c_str(), joinMessage.str().size(), 0);
        }
        catch (const std::bad_alloc&) 
        {
            std::cerr << "[ERROR] Failed to allocate memory for channel: " << params.channelName << std::endl;
            return;
        }
    }

    // Vérifier si le canal est en mode invite-only et si l'utilisateur est invité
    if (channel->isInviteOnly() && !channel->isUserInvited(nickname)) 
    {
        std::cout << "[DEBUG] FD " << params.client_fd << " (" << nickname 
                  << ") denied access to invite-only channel " << params.channelName << std::endl;

        std::ostringstream errorMsg;
        errorMsg << ":irc.server 473 " << params.client_fd << " " << params.channelName 
                 << " :Cannot join channel (Invite only)\n";
        send(params.client_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
        return;
    }

    // Afficher la liste des utilisateurs déjà présents dans le canal
    std::cout << "[DEBUG] Current users in " << params.channelName << " before joining: ";
    channel->listUsers();

    // Vérifier si l'utilisateur est déjà dans le canal
    if (!channel->isUserInChannel(nickname))
    {
        channel->addUser(params.client_fd, nickname);
        std::cout << "[DEBUG] FD " << params.client_fd << " (" << nickname << ") successfully joined channel " 
                  << params.channelName << std::endl;

        std::ostringstream joinMessage;
        joinMessage << ":irc.server 331 " << params.client_fd << " " << params.channelName << " :You have joined the channel\n";
        send(params.client_fd, joinMessage.str().c_str(), joinMessage.str().size(), 0);

        // Diffuser aux autres
        channel->broadcast(joinMessage.str(), params.client_fd);

        std::cout << "[DEBUG] Users in " << params.channelName << " after join: ";
        channel->listUsers();
    }
    else
    {
        std::cout << "[DEBUG] FD " << params.client_fd << " (" << nickname << ") is already in " 
                  << params.channelName << std::endl;

        std::ostringstream errorMsg;
        errorMsg << ":irc.server 443 " << params.client_fd << " " << nickname << " " 
                 << params.channelName << " :You're already in the channel\n";
        send(params.client_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
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
