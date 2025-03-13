/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:33:51 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/26 13:59:48 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
#include "../includes/Channel.hpp"

void Server::handleJoin(const CommandParams& params)
{
    Channel* channel = NULL;
    std::string nickname = getNickname(params.client_fd);

    // std::cout << " Client FD " << params.client_fd << " (" << nickname << ") trying to join channel " << params.channelName << std::endl;

    // Vérifier si le canal existe déjà
    for (size_t i = 0; i < channels.size(); ++i) 
    {
        if (channels[i]->getName() == params.channelName) 
        {
            channel = channels[i];
            break;
        }
    }

    // Si le canal n'existe pas encore, le créer
    if (!channel) 
    {
        try 
        {
            channel = new Channel(params.channelName);
            channels.push_back(channel);
            channel->addOperator(params.client_fd);

            // std::cout << " New channel created: " << params.channelName << " by FD " << params.client_fd << " (" << nickname << ")" << std::endl;

            sendMessage(params.client_fd, ":irc.server 332 " + params.channelName + " :Channel created\r\n");
        }
        catch (const std::bad_alloc&) 
        {
            sendError(params.client_fd, "500", "JOIN", "Internal server error");
            return;
        }
    }

    // Vérifier si l'utilisateur tente de rejoindre avec une mauvaise syntaxe
    if (params.password == "+k")  
    {
        sendError(params.client_fd, "461", "JOIN", "Invalid syntax. Use JOIN #channel [password]");
        // std::cout << " ❌ ERREUR : Mauvaise syntaxe JOIN #chat +k, attendu JOIN #chat pass." << std::endl;
        return;
    }

    // 🔥 Vérification si le canal est en mode +k (protégé par mot de passe)
    if (channel->hasKey())
    {
        // std::cout << " 🔒 Le canal " << params.channelName << " est protégé par un mot de passe." << std::endl;
        // std::cout << " 🔑 Mot de passe attendu : " << channel->getKey() << std::endl;
        // std::cout << " 🔑 Mot de passe reçu : " << params.password << std::endl;

        if (params.password.empty() || params.password != channel->getKey())
        {
            sendError(params.client_fd, "475", "JOIN", "Cannot join channel (+k) - Incorrect password");
            // std::cout << " ❌ Refus de connexion : mot de passe incorrect ou absent." << std::endl;

            // 🔥 Envoyer un KICK pour forcer Irssi à quitter complètement le canal
            std::ostringstream kickMessage;
            kickMessage << ":irc.server KICK " << params.channelName << " " << getNickname(params.client_fd)
                        << " :Authentication failed\r\n";
            sendMessage(params.client_fd, kickMessage.str());
            return;
        }
    }
    else
    {
        // std::cout << " ⚠️ Problème : hasKey() retourne FALSE alors que le mode +k devrait être actif !" << std::endl;
    }

    if (channel->isInviteOnly() && !channel->isUserInvited(nickname))// Vérifier si le canal est en mode invite-only et si l'utilisateur est invité
    {
        sendError(params.client_fd, "473", "JOIN", "Cannot join channel (Invite only)");
        // std::cout << " ❌ Refus de connexion : Not invited" << std::endl;

        std::ostringstream kickMessage;// 🔥 Envoyer un KICK pour forcer Irssi à quitter complètement le canal
        kickMessage << ":irc.server KICK " << params.channelName << " " << getNickname(params.client_fd) << " :Authentication failed\r\n";
        sendMessage(params.client_fd, kickMessage.str());
        return;
    }
    if (channel->hasUserLimit() && channel->getUserCount() >= channel->getUserLimit())
    {
        sendError(params.client_fd, "471", "JOIN", "Cannot join channel (+l) - User limit reached");
        // std::cout << " ❌ Refus de connexion : limite d'utilisateurs atteinte (" 
                // << channel->getUserLimit() << ")" << std::endl;
        std::ostringstream kickMessage;// 🔥 Envoyer un KICK pour forcer Irssi à quitter complètement le canal
        kickMessage << ":irc.server KICK " << params.channelName << " " << getNickname(params.client_fd) << " :Authentication failed\r\n";
        sendMessage(params.client_fd, kickMessage.str());
        return;
    }
    // Ajouter l'utilisateur s'il n'est pas déjà dans le canal
    if (!channel->isUserInChannel(nickname))
    {
        channel->addUser(params.client_fd, nickname);

        std::ostringstream joinAnnounce;
        joinAnnounce << ":" << nickname << " JOIN " << params.channelName << "\r\n";
        channel->broadcast(joinAnnounce.str(), params.client_fd);

        // std::cout << " FD " << params.client_fd << " (" << nickname << ") successfully joined channel " << params.channelName << std::endl;

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
        totalUsersMessage << ":irc.server 322 " << nickname << " " << params.channelName << " " << channel->getUserCount() << " :Users in channel\r\n";
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
