/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:55:54 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/22 15:29:41 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"
// #include "../includes/Client.hpp"
// #include "../includes/Channel.hpp"

std::string Server::getName(int client_fd)
{
	for (size_t i = 0; i < clients.size(); ++i) {
		Client* client = clients[i];
		if (client->getFd() == client_fd) {
			return client->getName();
		}
	}
	return ""; // utiliser une exception avec thrwo ????
}

std::string Server::getNickname(int client_fd)
{
	for (size_t i = 0; i < clients.size(); ++i) {
		Client* client = clients[i];
		if (client->getFd() == client_fd) {
			return client->getNickname();
		}
	}
	return "";
}

Channel* Server::getChannelByName(const std::string &name) {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == name) {
			return channels[i];
		}
	}
	return NULL;
}


/*____________________________________________________________________*/
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


/*___________________________________________________________*/

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


/*___________________________________________________________*/

void Server::handleKick(const CommandParams &params)
{
    Channel *channel = NULL;
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
        // Utilisation de std::ostringstream pour convertir l'entier en chaîne
        std::ostringstream errorMsg;
        errorMsg << ":irc.server 403 " << params.operator_fd << " " << params.channelName << " :No such channel\n";
        send(params.operator_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
        return;
    }

    if (!channel->isOperator(params.operator_fd))
    {
        // Utilisation de std::ostringstream pour convertir l'entier en chaîne
        std::ostringstream errorMsg;
        errorMsg << ":irc.server 481 " << params.operator_fd << " :Permission Denied- You're not a channel operator\n";
        send(params.operator_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
        return;
    }

    if (!channel->isUserInChannel(params.nickname))
    {
        // Utilisation de std::ostringstream pour convertir l'entier en chaîne
        std::ostringstream errorMsg;
        errorMsg << ":irc.server 441 " << params.operator_fd << " " << params.nickname << " " << params.channelName << " :They aren't on that channel\n";
        send(params.operator_fd, errorMsg.str().c_str(), errorMsg.str().size(), 0);
        return;
    }

    int kicked_fd = channel->getUserFdByNickname(params.nickname);

    // Message KICK envoyé au canal
    std::ostringstream kickMessage;
    kickMessage << ":" << getNickname(params.client_fd) << " KICK " 
                << params.channelName << " " << params.nickname << "\n";
    channel->broadcast(kickMessage.str(), params.client_fd);

    // Retirer l'utilisateur du canal
    channel->removeUser(kicked_fd);

    // Message envoyé à l'utilisateur éjecté
    std::ostringstream kickedMessage;
    kickedMessage << ":irc.server " << getNickname(params.client_fd) << " KICK "
                 << params.channelName << " " << params.nickname << "\n";
    send(kicked_fd, kickedMessage.str().c_str(), kickedMessage.str().size(), 0);

    // Message IRC indiquant l'éjection
    std::ostringstream kickedMessageIRC;
    kickedMessageIRC << ":irc.server 443 " << params.client_fd << " " << params.nickname 
                     << " " << params.channelName << " :You have been kicked from the channel by " 
                     << getNickname(params.client_fd) << "\n";
    send(kicked_fd, kickedMessageIRC.str().c_str(), kickedMessageIRC.str().size(), 0);

    // Affichage dans le terminal
    std::cout << "FD: " << params.client_fd << " kicked user " << params.nickname
              << " from channel " << params.channelName << std::endl;
}


void Server::handleMode(const CommandParams& params)
{
	Channel* channel = getChannelByName(params.channelName);

	if (!channel)
	{
		std::string errorMsg = ":" + getNickname(params.client_fd) +
							   " 403 " + params.channelName + " :No such channel\r\n";
		send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	if (!channel->isOperator(params.client_fd))
	{
		std::string errorMsg = ":" + getNickname(params.client_fd) +
							   " 482 " + params.channelName + " :You're not channel operator\r\n";
		send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	std::vector<std::string>::const_iterator argIt = params.Arg.begin();
	for (; argIt != params.Arg.end(); ++argIt)
	{
		const std::string& mode = *argIt;

		if (mode == "+i")
		{
			channel->setInviteOnly(true);
			std::string response = ":" + getNickname(params.client_fd) +
								   " MODE " + params.channelName + " +i\r\n";
			send(params.client_fd, response.c_str(), response.size(), 0);
		}
		else if (mode == "-i")
		{
			channel->setInviteOnly(false);
			std::string response = ":" + getNickname(params.client_fd) +
								   " MODE " + params.channelName + " -i\r\n";
			send(params.client_fd, response.c_str(), response.size(), 0);
		}
		else if (mode == "+k")
		{
			if (++argIt != params.Arg.end())
			{
				const std::string& key = *argIt;
				channel->setKey(key);
				std::string response = ":" + getNickname(params.client_fd) +
									   " MODE " + params.channelName + " +k " + key + "\r\n";
				send(params.client_fd, response.c_str(), response.size(), 0);
			}
			else
			{
				std::string errorMsg = ":" + getNickname(params.client_fd) +
									   " 461 " + params.channelName + " +k :Key is missing\r\n";
				send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
				return;
			}
		}
		else if (mode == "-k")
		{
			channel->clearKey();
			std::string response = ":" + getNickname(params.client_fd) +
								   " MODE " + params.channelName + " -k\r\n";
			send(params.client_fd, response.c_str(), response.size(), 0);
		}
		else if (mode == "+l")
		{
			if (++argIt != params.Arg.end())
			{
				int limit = std::atoi(argIt->c_str());
				channel->setUserLimit(limit);

				std::ostringstream oss;
				oss << " MODE " << params.channelName << " +l " << limit << "\r\n";
				std::string response = ":" + getNickname(params.client_fd) + oss.str();
				send(params.client_fd, response.c_str(), response.size(), 0);
			}
			else
			{
				std::string errorMsg = ":" + getNickname(params.client_fd) +
									   " 461 " + params.channelName + " +l :Limit is missing\r\n";
				send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
				return;
			}
		}
		else if (mode == "-l")
		{
			channel->clearUserLimit();
			std::string response = ":" + getNickname(params.client_fd) +
								   " MODE " + params.channelName + " -l\r\n";
			send(params.client_fd, response.c_str(), response.size(), 0);
		}
		else if (mode == "+o")
		{
			if (++argIt != params.Arg.end())
			{
				const std::string& nickname = *argIt;
				Client* targetClient = getClientByNickname(nickname);
				if (targetClient) 
				{
					channel->addOperator(targetClient->getFd());
					std::string response = ":" + getNickname(params.client_fd) +
										   " MODE " + params.channelName + " +o " + nickname + "\r\n";
					send(params.client_fd, response.c_str(), response.size(), 0);
				}
				else
				{
					std::string errorMsg = ":" + getNickname(params.client_fd) +
										   " 401 " + nickname + " :No such nick/channel\r\n";
					send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
				}
			}
			else
			{
				std::string errorMsg = ":" + getNickname(params.client_fd) +
									   " 461 " + params.channelName + " +o :Nickname is missing\r\n";
				send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
				return;
			}
		}
		else if (mode == "-o")
		{
			if (++argIt != params.Arg.end())
			{
				const std::string& nickname = *argIt;
				Client* targetClient = getClientByNickname(nickname);
				if (targetClient)
				{
					channel->removeOperator(targetClient->getFd());
					std::string response = ":" + getNickname(params.client_fd) +
										   " MODE " + params.channelName + " -o " + nickname + "\r\n";
					send(params.client_fd, response.c_str(), response.size(), 0);
				}
				else
				{
					std::string errorMsg = ":" + getNickname(params.client_fd) +
										   " 401 " + nickname + " :No such nick/channel\r\n";
					send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
				}
			}
			else
			{
				std::string errorMsg = ":" + getNickname(params.client_fd) +
									   " 461 " + params.channelName + " -o :Nickname is missing\r\n";
				send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
				return;
			}
		}
		else
		{
			std::string errorMsg = ":" + getNickname(params.client_fd) +
								   " 472 " + mode + " :is unknown mode char to me\r\n";
			send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		}
	}
}


Client* Server::getClientByNickname(const std::string& nickname)
{
	for (size_t i = 0; i < clients.size(); ++i)
	{
		if (clients[i]->getNickname() == nickname)
			return clients[i];
	}
	return NULL; // Aucun client trouvé
}

void Server::parseCommand(const std::string& message, int client_fd)
{
    // Nettoyer les caractères parasites (\r\n)
    std::string clearMessage = cleanMessage(message);

    std::istringstream iss(clearMessage);
    std::string command;
    iss >> command;

    // Convertir la commande en majuscules
    for (size_t i = 0; i < command.size(); ++i)
        command[i] = static_cast<char>(std::toupper(command[i]));

    std::cout << "[DEBUG] Received command: '" << command << "' from client FD: " << client_fd << std::endl;

    if (command == "PASS" || command == "NICK" || command == "USER")
    {
        handleConnectionCommands(command, iss, client_fd);
    }
	else if (command == "MODE")
	{
		std::string target, mode;
		iss >> target >> mode;

		if (target.empty())
		{
			std::string errorMsg = ":server 461 MODE :Not enough parameters\r\n"; // 461: ERR_NEEDMOREPARAMS
			send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		}
		else
		{
			// Réponse par défaut
			std::string response = ":server MODE " + target + " " + mode + "\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
			std::cout << "[DEBUG] MODE command received for target: " << target << ", mode: " << mode << std::endl;
		}
	}

    else if (command == "KICK")
    {
        CommandParams params;
        params.commandType = CommandParams::KICK;
        params.client_fd = client_fd;
        params.operator_fd = client_fd;
        iss >> params.channelName >> params.nickname;
        handleKick(params);
    }
    else if (command == "JOIN")
    {
        std::string channelName;
        iss >> channelName;

        if (channelName.empty())
        {
            sendError(client_fd, "461", "JOIN", "Not enough parameters");
            return;
        }
        else if (channelName[0] != '#')
        {
            sendError(client_fd, "476", "JOIN", "Invalid channel name");
            return;
        }

        CommandParams params;
        params.commandType = CommandParams::JOIN;
        params.client_fd = client_fd;
        params.channelName = channelName;

        handleJoin(params);
    }
    else if (command == "INVITE")
    {
        std::string nickname, channelName;
        iss >> nickname >> channelName;

        if (nickname.empty() || channelName.empty())
        {
            sendError(client_fd, "461", "INVITE", "Not enough parameters");
            return;
        }

        CommandParams params;
        params.commandType = CommandParams::INVIT;
        params.client_fd = client_fd;
        params.operator_fd = client_fd;
        params.channelName = channelName;
        params.nickname = nickname;

        handleInvit(params);
    }
    else
    {
        sendError(client_fd, "421", command, "Unknown command");
    }
}



void Server::handleConnectionCommands(const std::string& command, std::istringstream& iss, int client_fd)
{
	if (command == "PASS")
	{
		std::string password;
		iss >> password;

		if (password != this->password)
		{
			std::string errorMsg = ":server ERROR :Closing link: (user@host) [Invalid password]\r\n";
			send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
			close(client_fd); // Déconnexion après échec
			return;
		}

		std::cout << "[DEBUG] Client FD " << client_fd << " authenticated with password.\n";
	}
	else if (command == "NICK")
	{
		std::string nickname;
		iss >> nickname;

		if (nickname.empty())
		{
			std::string errorMsg = ":server 431 * :No nickname given\r\n"; // 431: ERR_NONICKNAMEGIVEN
			send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
			return;
		}

		if (nickname.find(' ') != std::string::npos || nickname.find(',') != std::string::npos)
		{
			std::string errorMsg = ":server 432 " + nickname + " :Erroneous nickname\r\n"; // 432: ERR_ERRONEOUSNICKNAME
			send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
			return;
		}

		for (size_t i = 0; i < clients.size(); ++i)
		{
			if (clients[i]->getNickname() == nickname)
			{
				std::string errorMsg = ":server 433 * " + nickname + " :Nickname is already in use\r\n"; // 433: ERR_NICKNAMEINUSE
				send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
				return;
			}
		}

		for (size_t i = 0; i < clients.size(); ++i)
		{
			if (clients[i]->getFd() == client_fd)
			{
				clients[i]->setNickname(nickname);
				std::cout << "[DEBUG] Client FD " << client_fd << " set nickname to " << nickname << std::endl;

				std::string nickResponse = ":" + nickname + " NICK " + nickname + "\r\n";
				broadcastToChannels(client_fd, nickResponse);
				return;
			}
		}
	}
	else if (command == "USER")
	{
		std::string username, hostname, servername, realname;
		iss >> username >> hostname >> servername;
		std::getline(iss, realname);

		if (!realname.empty() && realname[0] == ':')
		{
			realname = realname.substr(1);
		}

		if (username.empty() || realname.empty())
		{
			std::string errorMsg = ":server 461 USER :Not enough parameters\r\n"; // 461: ERR_NEEDMOREPARAMS
			send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
			return;
		}

		for (size_t i = 0; i < clients.size(); ++i)
		{
			if (clients[i]->getFd() == client_fd)
			{
				clients[i]->setName(realname);
				clients[i]->setUsername(username);
				std::cout << "[DEBUG] Client FD " << client_fd << " set username to " << username << std::endl;

				if (!clients[i]->getNickname().empty())
				{
					std::string welcomeMsg = ":server 001 " + clients[i]->getNickname() +
											 " :Welcome to the IRC server, " + clients[i]->getNickname() + "!\r\n"; // 001: RPL_WELCOME
					send(client_fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
					std::cout << "[DEBUG] Welcome message sent to client FD " << client_fd << std::endl;
				}
				return;
			}
		}
	}
	else if (command == "PING")
{
    std::string server;
    iss >> server;

    if (server.empty())
    {
        std::string errorMsg = ":server 409 :No origin specified\r\n"; // 409: ERR_NOORIGIN
        send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    std::string pongResponse = ":server PONG " + server + "\r\n";
    send(client_fd, pongResponse.c_str(), pongResponse.size(), 0);
    std::cout << "[DEBUG] Responded to PING from FD " << client_fd << std::endl;
}

	else if (command == "WHOIS")
{
    std::string targetNickname;
    iss >> targetNickname;

    if (targetNickname.empty())
    {
        std::string errorMsg = ":server 431 * :No nickname given\r\n"; // 431: ERR_NONICKNAMEGIVEN
        send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
        std::cout << "[DEBUG] WHOIS received with no nickname specified.\n";
        return;
    }

    bool found = false;
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getNickname() == targetNickname)
        {
            std::string whoisResponse = ":server 311 " + targetNickname + " " +
                                        clients[i]->getUsername() + " host :Real name: " +
                                        clients[i]->getName() + "\r\n"; // 311: RPL_WHOISUSER
            send(client_fd, whoisResponse.c_str(), whoisResponse.size(), 0);
            std::cout << "[DEBUG] WHOIS response sent for nickname: " << targetNickname << std::endl;

            found = true;
            break;
        }
    }

    if (!found)
    {
        std::string errorMsg = ":server 401 " + targetNickname + " :No such nick/channel\r\n"; // 401: ERR_NOSUCHNICK
        send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
        std::cout << "[DEBUG] WHOIS failed: no such nickname " << targetNickname << std::endl;
    }
}

	else
	{
		std::cerr << "[DEBUG] Commande non reconnue : " << command << std::endl;
		std::string errorMsg = ":server 421 " + command + " :Unknown command\r\n"; // 421: ERR_UNKNOWNCOMMAND
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
	}
}



void Server::handleOtherCommands(const std::string& command, std::istringstream& iss, int client_fd)
{
	if (command == "CAP")
{
    std::string subcommand;
    iss >> subcommand;

    if (subcommand == "LS")
    {
        // Réponse vide pour indiquer aucune capacité supplémentaire
        std::string response = ":server CAP * LS :\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
        std::cout << "[DEBUG] CAP LS handled for client FD " << client_fd << std::endl;
    }
    else
    {
        std::string response = ":server CAP * ACK :\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
    }
}
	else if (command == "TOPIC")
	{
		const char* errorMsg = "ERROR: TOPIC command is not implemented yet.\r\n";
		send(client_fd, errorMsg, strlen(errorMsg), 0);
		std::cout << "[DEBUG] TOPIC command received but not implemented.\n";
	}
	else if (command == "INVITE")
	{
		const char* errorMsg = "ERROR: INVITE command is not implemented yet.\r\n";
		send(client_fd, errorMsg, strlen(errorMsg), 0);
		std::cout << "[DEBUG] INVITE command received but not implemented.\n";
	}
	else
	{
		std::string errorMsg = "ERROR: Unknown command '" + command + "'.\r\n";
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
	}
}


void Server::parseModeOptions(const std::vector<std::string>& args, CommandParams& params)
{
	char modeType = 0; // Indique si on traite des modes à ajouter (+) ou retirer (-)

	for (size_t i = 0; i < args.size(); ++i)
	{
		const std::string& token = args[i];

		// Si le token commence par '+' ou '-', on met à jour le type de mode
		if (token[0] == '+' || token[0] == '-')
		{
			modeType = token[0];

			// Parcourir les caractères suivants pour identifier les modes
			for (size_t j = 1; j < token.size(); ++j)
			{
				char mode = token[j];

				// Ajouter ou retirer le mode dans la liste des paramètres
				if (modeType == '+')
					params.Arg.push_back("+" + std::string(1, mode));
				else if (modeType == '-')
					params.Arg.push_back("-" + std::string(1, mode));
			}
		}
		// Si le token est un argument (pas un indicateur de mode), on l'associe au dernier mode
		else 
		{
			if (!params.Arg.empty())
				params.additionalParams.push_back(token);
			else
				std::cerr << "[ERROR] Invalid MODE arguments: " << token << std::endl;
		}
	}
}


void Server::handleModeCommand(std::istringstream& iss, int client_fd)
{
	std::string channelName;
	iss >> channelName;

	if (channelName.empty())
	{
		std::string errorMsg = ":" + getNickname(client_fd) +
							   " 461 MODE :Not enough parameters\r\n";
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	Channel* channel = getChannelByName(channelName);

	// Vérifier si le canal existe
	if (!channel)
	{
		std::string errorMsg = ":" + getNickname(client_fd) +
							   " 403 " + channelName + " :No such channel\r\n";
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	// Vérifier si l'utilisateur est opérateur
	if (!channel->isOperator(client_fd))
	{
		std::string errorMsg = ":" + getNickname(client_fd) +
							   " 482 " + channelName + " :You're not channel operator\r\n";
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	// Extraire les modes et leurs paramètres
	std::vector<std::string> args;
	std::string arg;
	while (iss >> arg)
		args.push_back(arg);

	if (args.empty())
	{
		// Si aucun mode n'est spécifié, retourner les modes actuels
		std::string currentModes = channel->isInviteOnly() ? "+i" : "-i"; // Correction ici
		if (channel->hasKey()) currentModes += "+k";
		if (channel->getUserLimit() > 0) currentModes += "+l";
		if (channel->isTopicLock()) currentModes += "+t";

		std::string response = ":" + getNickname(client_fd) +
							   " 324 " + channelName + " " + currentModes + "\r\n";
		send(client_fd, response.c_str(), response.size(), 0);
		return;
	}

	// Traiter les modes via `handleMode`
	CommandParams params;
	params.commandType = CommandParams::MODE; // Correction ici
	params.client_fd = client_fd;
	params.channelName = channelName;
	params.Arg = args;

	handleMode(params);
}

std::string Server::cleanMessage(const std::string& message)
{
    std::string cleanMsg = message;
    while (!cleanMsg.empty() &&
           (cleanMsg[cleanMsg.size() - 1] == '\r' || cleanMsg[cleanMsg.size() - 1] == '\n'))
    {
        cleanMsg.erase(cleanMsg.size() - 1); // Supprime le dernier caractère
    }
    return cleanMsg;
}