/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:55:54 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/29 14:52:05 by thlefebv         ###   ########.fr       */
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

std::string Server::getNickname(int fd)
{
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getFd() == fd) // Vérifier avec le bon FD
            return clients[i]->getNickname();
    }
    return "*"; // Retourne "*" si aucun client trouvé
}


Channel* Server::getChannelByName(const std::string &name) {
	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == name) {
			return channels[i];
		}
	}
	return NULL;
}

/*------------------------------------------------*/

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

    if (command == "PASS" || command == "NICK" || command == "USER" || command == "CAP")
    {
        handleConnexionCommands(command, iss, client_fd);
        return;
    }

    // Vérifier si le client est enregistré avant de traiter d'autres commandes
    if (notregistered(client_fd))
    {
        sendError(client_fd, "451", command, "You have not registered"); // 451: ERR_NOTREGISTERED
        return;
    }

    if (command == "MODE")
    {
        std::string target, mode;
        iss >> target >> mode;

        if (target.empty())
        {
            sendError(client_fd, "461", "MODE", "Not enough parameters");
            return;
        }
        std::string response = ":server MODE " + target + " " + mode + "\r\n";
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }
    else if (command == "KICK")
    {
        CommandParams params;
        params.commandType = CommandParams::KICK;
        params.client_fd = client_fd;
        params.operator_fd = client_fd;
        iss >> params.channelName >> params.nickname;
        handleKick(params);
        return;
    }
    else if (command == "JOIN")
    {
        std::string channelName;
        iss >> channelName;

        if (channelName.empty() || channelName[0] != '#')
        {
            sendError(client_fd, "476", "JOIN", "Invalid channel name");
            return;
        }

        CommandParams params;
        params.commandType = CommandParams::JOIN;
        params.client_fd = client_fd;
        params.channelName = channelName;

        handleJoin(params);
        return;
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
        return;
    }
    else
    {
        sendError(client_fd, "421", command, "Unknown command");
    }
}


void Server::handleConnexionCommands(const std::string& command, std::istringstream& iss, int client_fd)
{
	if(command == "CAP")
	{
		handleOtherCommands(command, iss, client_fd);
		return;
	}
	if (command == "PASS")
	{
		std::string password;
		iss >> password;

		if (password != this->password)
		{
			std::string errorMsg = ":server 464 * :Password incorrect\r\n"; // IRC standard: ERR_PASSWDMISMATCH
			send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
			close(client_fd); // Déconnexion après échec
			return;
		}

		std::cout << "[DEBUG] Client FD " << client_fd << " authenticated with password.\n";

		// ⚠️ Important : Envoyer un message de confirmation à Irssi
		std::string passConfirm = ":server NOTICE * :Password accepted, proceed with NICK and USER\r\n";
		send(client_fd, passConfirm.c_str(), passConfirm.size(), 0);
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
			realname = realname.substr(1);
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
					sendWelcomeMessage(client_fd);
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
			std::string response = ":irc.server CAP * LS :\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
			std::cout << "[DEBUG] CAP LS handled for client FD " << client_fd << std::endl;
		}
		else
		{
			std::string response = ":irc.server CAP * ACK :\r\n";
			send(client_fd, response.c_str(), response.size(), 0);
			std::cout << "[DEBUG] CAP " << subcommand << " acknowledged for client FD " << client_fd << std::endl;
		}
	}
	else
	{
		std::string errorMsg = "ERROR: Unknown command '" + command + "'.\r\n";
		send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
	}
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