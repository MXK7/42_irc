/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 15:21:03 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/26 13:59:48 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::handleKick(const CommandParams &params)
{
    Channel *channel = getChannelByName(params.channelName);
    if (!channel)
    {
        sendError(params.operator_fd, "403", params.channelName, "No such channel");
        return;
    }

    Client* operatorClient = getClientByFd(params.operator_fd);
    if (!operatorClient)
    {
        // std::cout << " ❌ Erreur: Client opérateur non trouvé!" << std::endl;
        return;
    }

    std::string operatorNick = operatorClient->getNickname();
    if (!channel->isUserInChannel(operatorNick))
    {
        sendError(params.operator_fd, "441", operatorNick, "You're not on that channel");
        return;
    }

    if (!channel->isOperator(params.operator_fd))
    {
        // 🔥 Envoyer un message d'erreur au canal si l'utilisateur n'est pas opérateur
        std::ostringstream kickErrorMessage;
        kickErrorMessage << ":" << operatorNick << "!user@irc.server PRIVMSG " << params.channelName
                         << " :" << operatorNick
                         << " tried to kick a user but is not an operator.\r\n";

        channel->broadcast(kickErrorMessage.str());
        return;
    }

    if (!channel->isUserInChannel(params.nickname))
    {
        sendError(params.operator_fd, "441", params.nickname, "They aren't on that channel");
        return;
    }

    int kicked_fd = channel->getUserFdByNickname(params.nickname);
    Client* kickedClient = getClientByFd(kicked_fd);
    if (!kickedClient)
    {
        // std::cout << " ❌ Erreur: Client à kicker non trouvé!" << std::endl;
        return;
    }

    // Construire le message KICK avec le préfixe complet
    std::ostringstream kickMessage;
    kickMessage << ":" << operatorNick << "!" << operatorClient->getUsername()
                << "@" << operatorClient->getHostname() << " KICK "
                << params.channelName << " " << params.nickname
                << " :" << (params.message.empty() ? "Kicked from channel" : params.message) << "\r\n";

    // Diffuser le message KICK
    channel->broadcast(kickMessage.str());

    // Retirer l'utilisateur du canal
    if (channel->isOperator(kicked_fd))
        channel->removeOperator(kicked_fd);
    channel->removeUser(kicked_fd);

    // std::cout << " FD " << params.operator_fd << " kicked user "
            //   << params.nickname << " from channel " << params.channelName << std::endl;

    // Vérifier si le canal est vide et le supprimer si nécessaire
    if (channel->getUserCount() == 0)
    {
        removeChannel(params.channelName);
    }
}

void Server::removeChannel(const std::string &channelName)
{
    for (std::vector<Channel*>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        if ((*it)->getName() == channelName)
        {
            delete *it; // 🔥 Libère la mémoire
            channels.erase(it);
            // std::cout << " 🔥 Suppression du canal vide : " << channelName << std::endl;
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
