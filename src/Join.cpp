/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:33:51 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/28 10:34:18 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::handleJoin(const CommandParams& params)
{
    Channel* channel = NULL;

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

            channel->addOperator(params.client_fd); // Ajouter l'opérateur
            std::cout << "New channel created: " << params.channelName << std::endl;

            // Message IRC de confirmation de création de canal
            std::ostringstream joinMessage;
            joinMessage << ":irc.server 332 " << params.client_fd << " " << params.channelName << " :Channel created\n";
            send(params.client_fd, joinMessage.str().c_str(), joinMessage.str().size(), 0);
        }
        catch (const std::bad_alloc&) 
        {
            std::cerr << "ERROR: Failed to allocate memory for channel.\n";
            return;
        }
    }

    // Vérifier si le canal est en mode invite-only et l'utilisateur est invité
    if (channel->isInviteOnly() && !channel->isUserInvited(params.nickname)) 
    {
        // Message d'erreur si le canal est en mode invite-only
        std::ostringstream errorMsg;
        errorMsg << ":irc.server 473 " << params.client_fd << " " << params.channelName << " :Cannot join channel (Invite only)\n";
        send(params.client_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
        return;
    }

    // Ajouter l'utilisateur au canal
    if (!channel->isUserInChannel(params.nickname)) 
    {
        channel->addUser(params.client_fd, params.nickname);
        std::cout << "FD: " << params.client_fd << " joined channel " << params.channelName << std::endl;

        // Message de confirmation de la connexion au canal
        std::ostringstream joinMessage;
        joinMessage << ":irc.server 331 " << params.client_fd << " " << params.channelName << " :You have joined the channel\n";
        send(params.client_fd, joinMessage.str().c_str(), joinMessage.str().size(), 0);

        // Diffuser le message aux autres utilisateurs du canal
        channel->broadcast(joinMessage.str(), params.client_fd);
    }
    else
    {
        // Si l'utilisateur est déjà dans le canal, envoyer un message informatif
        std::ostringstream errorMsg;
        errorMsg << ":irc.server 443 " << params.client_fd << " " << params.nickname << " " << params.channelName << " :You're already in the channel\n";
        send(params.client_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
    }
}