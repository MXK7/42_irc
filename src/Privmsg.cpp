/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/19 09:23:41 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/19 10:07:21 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::handlePrivMsg(const CommandParams &params)
{
    // Vérifier si un destinataire et un message sont présents
    if (params.Arg.size() < 2)
    {
        sendError(params.client_fd, "461", "PRIVMSG", "Not enough parameters"); // 461: ERR_NEEDMOREPARAMS
        return;
    }

    std::string target = params.Arg[0];  // Destinataire (nick ou #channel)
    std::string message = params.Arg[1]; // Message privé

    // 🔎 Vérifier si le message est vide
    if (message.empty())
    {
        sendError(params.client_fd, "412", "PRIVMSG", "No text to send"); // 412: ERR_NOTEXTTOSEND
        return;
    }

    std::string senderNick = getNickname(params.client_fd);
    std::ostringstream privMsg;
    privMsg << ":" << senderNick << " PRIVMSG " << target << " :" << message << "\r\n";

    // 🔹 Si le message est destiné à un canal
    if (target[0] == '#')
    {
        Channel *channel = getChannelByName(target);
        if (!channel)
        {
            sendError(params.client_fd, "403", target, "No such channel"); // 403: ERR_NOSUCHCHANNEL
            return;
        }

        // Vérifier si l'expéditeur est bien dans le canal
        if (!channel->isUserInChannel(senderNick))
        {
            sendError(params.client_fd, "404", target, "Cannot send to channel"); // 404: ERR_CANNOTSENDTOCHAN
            return;
        }

        // 🔥 Diffuser le message dans le canal (sauf à l'expéditeur)
        channel->broadcast(privMsg.str(), params.client_fd);
    }
    else // 🔹 Message privé à un utilisateur
    {
        int recipient_fd = getClientFdByNickname(target);
        if (recipient_fd == -1)
        {
            sendError(params.client_fd, "401", target, "No such nick"); // 401: ERR_NOSUCHNICK
            return;
        }

        // 🔥 Envoyer le message privé au destinataire
        sendMessage(recipient_fd, privMsg.str());
    }
}


