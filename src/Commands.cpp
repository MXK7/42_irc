/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/12 10:55:54 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/16 13:16:40 by thlefebv         ###   ########.fr       */
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
        std::string errorMsg = "ERROR: Channel is invite-only.\n";
        send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    // Ajouter l'utilisateur au canal
    if (!channel->isUserInChannel(params.nickname)) 
    {
        channel->addUser(params.client_fd, params.nickname);
        std::cout << "FD: " << params.client_fd << " joined channel " << params.channelName << std::endl;

        // Envoyer un message de confirmation
        std::string joinMessage = ":" + getNickname(params.client_fd) + " JOIN " + params.channelName + "\n";
        send(params.client_fd, joinMessage.c_str(), joinMessage.size(), 0);

        // Diffuser le message aux autres utilisateurs du canal
        channel->broadcast(joinMessage, params.client_fd);
    }
}

/*___________________________________________________________*/

void Server::handleInvit(const CommandParams &params)
{
	Channel *channel = NULL;

	for (size_t i = 0; i < channels.size(); ++i) {
		if (channels[i]->getName() == params.channelName) {
			channel = channels[i];
			break;
		}
	}
		// ?? gestion mess d erreur
	if (!channel) {
		std::cerr << "Error: Channel " << params.channelName
					<< " does not exist!" << std::endl;
		return;
	}

	if (!channel->isOperator(params.operator_fd)) {
		std::string errorMsg = "ERROR : You are not an operator in channel "
							+ params.channelName + ".\n";
		send(params.operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	Client *targetClient = NULL;
	for (size_t i = 0; i < clients.size(); ++i) {
		if (clients[i]->getNickname() == params.nickname) {
			targetClient = clients[i];
			break;
		}
	}

	if (!targetClient) {
		std::string errorMsg = "ERROR : User " +
							params.nickname + " not found.\n";
		send(params.operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	channel->addInvitedUser(params.nickname);

	std::string inviteMsg = ":" + getNickname(params.operator_fd) +
						" INVITE " + params.nickname + " " + params.channelName + "\n";
	send(targetClient->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);

	std::string confirmationMsg = "You have invited " + params.nickname +
							" to channel " + params.channelName + ".\n";
	send(params.operator_fd, confirmationMsg.c_str(), confirmationMsg.size(), 0);

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

	if (!channel) {
		std::cerr << "Error: Channel " << params.channelName
					<< " does not exist!" << std::endl;
		return;
	}

	 if (!channel->isOperator(params.operator_fd)) {
		std::cerr << "Error: You are not an operator in this channel!" << std::endl;
		std::string errorMsg = "You are not an operator in this channel.\n";
		send(params.operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	if (!channel->isUserInChannel(params.nickname)) {
		std::cerr << "Error: User " << params.nickname
					 << " is not in the channel!" << std::endl;
		std::string errorMsg = "User " + params.nickname + " is not in the channel.\n";
		send(params.operator_fd, errorMsg.c_str(), errorMsg.size(), 0);
		return;
	}

	int kicked_fd = channel->getUserFdByNickname(params.nickname);

	std::string kickMessage = ":" + getNickname(params.client_fd) + " KICK "
						+ params.channelName + " " + params.nickname + "\n";
	channel->broadcast(kickMessage, params.client_fd);

	channel->removeUser(kicked_fd);

	std::string kickedMessage = "You have been kicked from " + params.channelName
						+ " by " + getNickname(params.client_fd) + "\n";
	send(kicked_fd, kickedMessage.c_str(), kickedMessage.size(), 0);

	std::cout << "FD: " << params.client_fd << " kicked user " << params.nickname
						<< " from channel " << params.channelName << std::endl;
}


void Server::parseCommand(const std::string& message, int client_fd)
{
    std::istringstream iss(message);
    std::string command;
    iss >> command;

    // Convertir la commande en majuscules
    for (size_t i = 0; i < command.size(); ++i)
        command[i] = std::toupper(command[i]);

    std::cout << "[DEBUG] Received command: '" << command << "' from client FD: " << client_fd << std::endl;

    if (command == "CAP") {
        std::string subcommand;
        iss >> subcommand;
        if (subcommand == "LS") {
            const char* response = "CAP * LS :\r\n"; // Aucune capacité supportée
            send(client_fd, response, strlen(response), 0);
            std::cout << "[DEBUG] CAP LS response sent to client FD " << client_fd << std::endl;
        } else if (subcommand == "END") {
            std::cout << "[DEBUG] CAP negotiation ended for client FD " << client_fd << std::endl;
        }
        return;
    }

    if (command == "NICK") {
        std::string nickname;
        iss >> nickname;

        if (nickname.empty()) {
            const char* errorMsg = "ERROR: NICK command requires a nickname.\r\n";
            send(client_fd, errorMsg, strlen(errorMsg), 0);
            return;
        }

        // Vérifiez si le pseudonyme existe déjà
        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i]->getNickname() == nickname) {
                std::string errorMsg = "ERROR: Nickname '" + nickname + "' is already in use.\r\n";
                send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
                return;
            }
        }

        // Attribuez le pseudonyme au client
        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i]->getFd() == client_fd) {
                clients[i]->setNickname(nickname);
                std::cout << "[DEBUG] Client FD " << client_fd << " set nickname to " << nickname << std::endl;

                // Confirmez le changement de pseudonyme
                std::string nickResponse = ":" + nickname + " NICK " + nickname + "\r\n";
                send(client_fd, nickResponse.c_str(), nickResponse.size(), 0);
                return;
            }
        }
    }

    if (command == "USER") {
        std::string username, hostname, servername, realname;
        iss >> username >> hostname >> servername >> std::ws;
        std::getline(iss, realname);

        if (username.empty() || realname.empty()) {
            const char* errorMsg = "ERROR: USER command requires a username and a realname.\r\n";
            send(client_fd, errorMsg, strlen(errorMsg), 0);
            return;
        }

        for (size_t i = 0; i < clients.size(); ++i) {
            if (clients[i]->getFd() == client_fd) {
                clients[i]->setName(realname); // Définir le nom réel
                clients[i]->setUsername(username);

                std::cout << "[DEBUG] Client FD " << client_fd << " set username to " << username << std::endl;

                // Si le pseudonyme est défini, envoyez un message de bienvenue
                if (!clients[i]->getNickname().empty()) {
                    std::string welcomeMsg = ":server 001 " + clients[i]->getNickname() +
                                             " :Welcome to the IRC server, " + clients[i]->getNickname() + "!\r\n";
                    send(client_fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);
                    std::cout << "[DEBUG] Welcome message sent to client FD " << client_fd << std::endl;
                }
                return;
            }
        }
    }
    if (command == "JOIN") {
    const char* errorMsg = "ERROR: JOIN command is not implemented yet.\r\n";
    send(client_fd, errorMsg, strlen(errorMsg), 0);
    std::cout << "[DEBUG] JOIN command received but not implemented.\n";
    return;
}
    // Commande inconnue
    else {
        std::string errorMsg = "ERROR: Unknown command '" + command + "'.\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
    }
}