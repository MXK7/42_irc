/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:31:36 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/20 18:15:02 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::handleInvit(const CommandParams &params)
{
    // Vérifier si le canal existe
    Channel *channel = NULL;
    for (size_t i = 0; i < channels.size(); ++i)
    {
        if (channels[i]->getName() == params.channelName)
        {
            channel = channels[i];
            break;
        }
    }

    // Message d'erreur : canal introuvable (ERR_NOSUCHCHANNEL - 403)
    if (!channel)
    {
        std::ostringstream errorMsg;
        errorMsg << ":irc.server 403 " << params.operator_fd << " " 
                 << params.channelName << " :No such channel\n";
        send(params.operator_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
        return;
    }

    // Vérifier si l'utilisateur est opérateur du canal
    if (!channel->isOperator(params.operator_fd))
    {
        // Message d'erreur : non-opérateur (ERR_CHANOPRIVSNEEDED - 482)
        std::ostringstream errorMsg;
        errorMsg << ":irc.server 482 " << params.operator_fd << " " 
                 << params.channelName << " :You're not a channel operator\n";
        send(params.operator_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
        return;
    }

    // Vérifier si l'utilisateur ciblé existe
    Client *targetClient = NULL;
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getNickname() == params.nickname)
        {
            targetClient = clients[i];
            break;
        }
    }

    // Message d'erreur : utilisateur introuvable (ERR_NOSUCHNICK - 401)
    if (!targetClient)
    {
        std::ostringstream errorMsg;
        errorMsg << ":irc.server 401 " << params.operator_fd << " " 
                 << params.nickname << " :No such nick/channel\n";
        send(params.operator_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
        return;
    }

    // Ajouter l'utilisateur à la liste des invités
    channel->addInvitedUser(params.nickname);

    // Envoyer le message d'invitation à l'utilisateur ciblé
    std::ostringstream inviteMsg;
    inviteMsg << ":" << getNickname(params.operator_fd) << " INVITE " 
              << params.nickname << " :" << params.channelName << "\n";
    send(targetClient->getFd(), inviteMsg.str().c_str(), inviteMsg.str().size(), 0);

    // Confirmation pour l'opérateur
    std::ostringstream confirmationMsg;
    confirmationMsg << ":irc.server 341 " << params.operator_fd << " " 
                    << params.nickname << " " << params.channelName << "\n";
    send(params.operator_fd, confirmationMsg.str().c_str(), confirmationMsg.str().size(), 0);

    // Log pour le terminal
    std::cout << "Operator FD " << params.operator_fd << " invited " 
              << params.nickname << " to channel " << params.channelName << std::endl;
}
