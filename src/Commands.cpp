/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:55:54 by vmassoli          #+#    #+#             */
/*   Updated: 2025/02/13 17:39:50 by thlefebv         ###   ########.fr       */
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
        std::cout << "[DEBUG] 🔍 Comparaison : " << clients[i]->getNickname() << " == " << nickname << std::endl;
        if (clients[i]->getNickname() == nickname)
            return clients[i];
    }
    return NULL;
}


void Server::parseCommand(const std::string& message, int client_fd)
{
	std::string clearMessage = cleanMessage(message);

	std::istringstream iss(clearMessage);
	std::string command;
	iss >> command;

	std::cout << "[DEBUG COMMANDE] Commande brute reçue: " << message << std::endl;

	// Convertir la commande en majuscules
	for (size_t i = 0; i < command.size(); ++i)
		command[i] = static_cast<char>(std::toupper(command[i]));

	// std::cout << "[DEBUG] Received command: '" << command << "' from client FD: " << client_fd << std::endl;

	if (command == "PASS" || command == "NICK" || command == "USER" || command == "CAP" || command == "PING" || command == "PRIVMSG")
	{
		handleConnexionCommands(command, iss, client_fd);
		return;
	}

	if (notregistered(client_fd))
	{
		sendError(client_fd, "451", command, "You have not registered"); // 451: ERR_NOTREGISTERED
		return;
	}

	if (command == "MODE")
	{
		CommandParams params;
		params.commandType = CommandParams::MODE;
		params.client_fd = client_fd;

		// Lire le nom du canal
		if (!(iss >> params.channelName))
		{
			sendError(client_fd, "461", "MODE", "Not enough parameters");
			return;
		}

		// Lire les modes et arguments suivants
		std::string mode;
		while (iss >> mode)
		{
			params.Arg.push_back(mode);
		}

		// 🔥 DEBUG : Afficher les arguments extraits
		std::cout << "[DEBUG] 🛠️ Arguments extraits pour MODE : ";
		for (size_t i = 0; i < params.Arg.size(); i++)
			std::cout << "\"" << params.Arg[i] << "\" ";
		std::cout << std::endl;

		handleMode(params);
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
		CommandParams params;
		params.commandType = CommandParams::JOIN;
		params.client_fd = client_fd;

		iss >> params.channelName; // Récupère le nom du canal

		std::string possiblePassword;
		if (iss >> possiblePassword) // Récupère le mot de passe s'il existe
		{
			params.password = possiblePassword;
			std::cout << "[DEBUG] Mot de passe extrait pour JOIN : " << params.password << std::endl;
		}
		else
		{
			std::cout << "[DEBUG] Aucun mot de passe extrait pour JOIN." << std::endl;
		}

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
		return;
	}
	else if (command == "TOPIC")
	{
		std::string channelName, topic;
		iss >> channelName;
		std::getline(iss, topic);

		if (!topic.empty() && topic[0] == ':')
			topic = topic.substr(1);

		CommandParams params;
		params.commandType = CommandParams::TOPIC;
		params.client_fd = client_fd;
		params.channelName = channelName;
		if (!topic.empty()) params.Arg.push_back(topic);

		handleTopic(params);
		return;
	}
	else if(command == "PART")
	{
		CommandParams params;
		params.commandType = CommandParams::PART;
		params.client_fd = client_fd;

		// Récupérer le nom du canal
		if (!(iss >> params.channelName))
		{
			sendError(client_fd, "461", "PART", "Not enough parameters");
			return;
		}

		// Lire un message optionnel
		std::string message;
		if (std::getline(iss, message) && !message.empty() && message[0] == ':') 
		{
			// Le message commence par ":", donc on doit l'utiliser en entier
			params.message = message.substr(1);  // Supprimer le ":" initial
		}

		// Affichage des paramètres extraits pour debug
		std::cout << "[DEBUG] 🛠️ PARAMS pour PART : Canal = " << params.channelName;
		if (!params.message.empty()) 
		{
			std::cout << ", Message = " << params.message;
		}
		std::cout << std::endl;

		handlePart(params);  // Appel de la fonction pour gérer le départ du canal
		return;
	}
	else
		sendError(client_fd, "421", command, "Unknown command");
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
			sendError(client_fd, "464", "PASS", "Password incorrect"); // 464: ERR_PASSWDMISMATCH
			close(client_fd); // Déconnexion après échec
			return;
		}

		std::cout << "[DEBUG] Client FD " << client_fd << " authenticated with password.\n";
		sendMessage(client_fd, ":server NOTICE * :Password accepted, proceed with NICK and USER\r\n");// ⚠️ Important : Envoyer un message de confirmation à Irssi
	}

else if (command == "NICK")
{
    std::string newNickname;
    iss >> newNickname;

    if (newNickname.empty())
    {
        sendError(client_fd, "431", "NICK", "No nickname given"); // 431: ERR_NONICKNAMEGIVEN
        return;
    }

    if (newNickname.find(' ') != std::string::npos || newNickname.find(',') != std::string::npos)
    {
        sendError(client_fd, "432", "NICK", "Erroneous nickname"); // 432: ERR_ERRONEOUSNICKNAME
        return;
    }

    // Vérifier si le pseudonyme est déjà utilisé
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getNickname() == newNickname)
        {
            sendError(client_fd, "433", "NICK", "Nickname is already in use"); // 433: ERR_NICKNAMEINUSE
            return;
        }
    }

    // Récupérer l'ancien pseudo du client
    std::string oldNickname = getNickname(client_fd);
    
    // Mise à jour du pseudo dans la liste des clients
    for (size_t i = 0; i < clients.size(); ++i)
    {
        if (clients[i]->getFd() == client_fd)
        {
            clients[i]->setNickname(newNickname);
            std::cout << "[DEBUG] Client FD " << client_fd << " changed nickname from " 
                      << oldNickname << " to " << newNickname << std::endl;

            // ✅ Envoi de la réponse NICK au client
            std::string nickResponse = ":" + oldNickname + " NICK " + newNickname + "\r\n";
            sendMessage(client_fd, nickResponse);  // 🔥 Envoi du message de confirmation au client

            // Notification aux autres clients dans les canaux où l'utilisateur est
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
			sendError(client_fd, "461", "USER", "Not enough parameters");
			return;
		}

		for (size_t i = 0; i < clients.size(); ++i)
		{
			if (clients[i]->getFd() == client_fd)
			{
				clients[i]->setName(realname);
				clients[i]->setUsername(username);
				// std::cout << "[DEBUG] Client FD " << client_fd << " set username to " << username << std::endl;

				if (!clients[i]->getNickname().empty())
				{
					sendWelcomeMessage(client_fd);
					// std::cout << "[DEBUG] Welcome message sent to client FD " << client_fd << std::endl;
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
			sendError(client_fd, "409", "PING", "No origin specified"); // 409: ERR_NOORIGIN
			return;
		}

		std::string pongResponse = ":server PONG " + server + "\r\n";
		sendMessage(client_fd, pongResponse);
		// std::cout << "[DEBUG] Responded to PING from FD " << client_fd << std::endl;
	}
	else if (command == "PRIVMSG")
	{
		std::string target, message;
		iss >> target;
		std::getline(iss, message);

		if (target.empty() || message.empty())
		{
			sendError(client_fd, "412", "PRIVMSG", "No text to send"); // 412: ERR_NOTEXTTOSEND
			return;
		}

		if (!message.empty() && message[0] == ':')
			message = message.substr(1);

		std::string senderNickname = getNickname(client_fd);
		std::string fullMessage = ":" + senderNickname + " PRIVMSG " + target + " :" + message + "\r\n";

		if (target[0] == '#') // 🔥 Vérification stricte pour les messages de canal
		{
			Channel* channel = findChannel(target);
			
			if (!channel)  // 🔥 Le canal n'existe plus
			{
				sendError(client_fd, "403", "PRIVMSG", "No such channel"); // 403: ERR_NOSUCHCHANNEL
				return;
			}

			if (!channel->isUserInChannel(senderNickname)) // 🔥 Vérification si l'utilisateur est encore dans le canal
			{
				sendError(client_fd, "404", "PRIVMSG", "Cannot send to channel"); // 404: ERR_CANNOTSENDTOCHAN
				return;
			}

			channel->broadcast(fullMessage, client_fd); // ✅ Si tout est bon, envoyer le message
		}
		else // 🔥 Gestion des messages privés entre utilisateurs
		{
			int target_fd = getClientFdByNickname(target);
			if (target_fd != -1)
				sendMessage(target_fd, fullMessage);
			else
				sendError(client_fd, "401", "PRIVMSG", "No such nick"); // 401: ERR_NOSUCHNICK
		}

		std::cout << "[DEBUG] PRIVMSG from " << senderNickname << " to " << target << ": " << message << std::endl;
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
			sendMessage(client_fd, ":irc.server CAP * LS :\r\n");
			// std::cout << "[DEBUG] CAP LS handled for client FD " << client_fd << std::endl;
		}
		else
		{
			sendMessage(client_fd, ":irc.server CAP * ACK :\r\n");
			// std::cout << "[DEBUG] CAP " << subcommand << " acknowledged for client FD " << client_fd << std::endl;
		}
	}
	else
	{
		sendError(client_fd, "421", command, "Unknown command");
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