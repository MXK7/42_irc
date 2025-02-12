/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:25:46 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/12 09:46:13 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::sendModeResponse(int client_fd, const std::string& nickname, const std::string& channelName, const std::string& mode, const std::string& extra)
{
	std::string response = ":" + nickname + " MODE " + channelName + " " + mode;
	if (!extra.empty()) {
		response += " " + extra;
	}
	response += "\r\n";
	send(client_fd, response.c_str(), response.size(), 0);
}

void Server::handleMode(const CommandParams& params)
{
	
	Channel* channel = getChannelByName(params.channelName);
	if (!channel)
	{
		sendError(params.client_fd, "403", "MODE", "No such channel");
		return;
	}

	std::vector<std::string>::const_iterator argIt = params.Arg.begin();
	for (; argIt != params.Arg.end(); ++argIt)
	{
		const std::string& mode = *argIt;

		if (mode == "+i")
		{
			channel->setInviteOnly(true);
			sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "+i", "");
		}
		else if (mode == "-i")
		{
			channel->setInviteOnly(false);
			sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "-i", "");
		}
		else if (mode == "+k")  
		{
			if (++argIt != params.Arg.end())
			{
				std::string key = *argIt;
				channel->setKey(key);

				if (channel->hasKey())
					std::cout << "[DEBUG] ✅ Mode +k activé sur " << params.channelName << " avec clé : " << key << std::endl;
				else
					std::cout << "[DEBUG] ❌ ERREUR : setKey() n'a pas activé hasKey() !" << std::endl;

				sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "+k", key);
			}
			else
			{
				sendError(params.client_fd, "461", "MODE", "Not enough parameters for +k");
				return;
			}
		}
		else if (mode == "-k")  
		{
			channel->clearKey();
			std::cout << "[DEBUG] Mode -k : suppression du mot de passe sur " << params.channelName << std::endl;
			sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "-k", "");
		}
		else if (mode == "+l")
		{
			if (++argIt != params.Arg.end())  // 🔥 Vérifie s'il y a un argument
			{
				int limit = std::atoi(argIt->c_str());
				if (limit > 0)
				{
					channel->setUserLimit(limit);
					std::ostringstream modeMessage;
					modeMessage << ":irc.server MODE " << params.channelName << " +l " << limit << "\r\n";
					channel->broadcast(modeMessage.str());  // 🔥 Informe tout le canal
					std::cout << "[DEBUG] ✅ Limite d'utilisateurs de " << params.channelName << " fixée à " << limit << std::endl;
				}
				else
				{
					sendError(params.client_fd, "461", "MODE", "Invalid limit");
				}
			}
			else
			{
				sendError(params.client_fd, "461", "MODE", "Limit required for +l");
			}
		}
		else if (mode == "-l")
		{
			channel->clearUserLimit();
			std::ostringstream modeMessage;
			modeMessage << ":irc.server MODE " << params.channelName << " -l\r\n";
			channel->broadcast(modeMessage.str());
			std::cout << "[DEBUG] 🔓 Limite d'utilisateurs supprimée pour " << params.channelName << std::endl;
		}
		else if (mode == "+o")  
		{
			if (params.Arg.size() < 2)  // 🔥 On vérifie qu'on a bien "+o" suivi d'un nickname
			{
				sendError(params.client_fd, "461", "MODE", "Nickname required for +o");
				return;
			}

			const std::string& nickname = params.Arg[1];  // 🔥 Maintenant on prend bien le vrai nickname
			Client* targetClient = getClientByNickname(nickname);

			std::cout << "[DEBUG] 🔍 Recherche du client : " << nickname << std::endl;

			if (!targetClient)
			{
				std::cout << "[DEBUG] ❌ ERREUR : Client " << nickname << " introuvable !" << std::endl;
				sendError(params.client_fd, "401", "MODE", "No such nick/channel");
				return;
			}

			if (!channel->isUserInChannel(nickname))
			{
				sendError(params.client_fd, "441", "MODE", nickname + " is not on " + params.channelName);
				return;
			}

			if (!channel->isOperator(params.client_fd))  
			{
				sendError(params.client_fd, "482", "MODE", "You're not a channel operator");
				return;
			}

			if (channel->isOperator(targetClient->getFd()))
			{
				sendError(params.client_fd, "443", "MODE", nickname + " is already an operator");
				return;
			}

			// ✅ Ajout de l'opérateur
			channel->addOperator(targetClient->getFd());

			if (channel->isOperator(targetClient->getFd()))
				std::cout << "[DEBUG] ✅ " << nickname << " est maintenant opérateur de " << params.channelName << std::endl;
			else
				std::cout << "[DEBUG] ❌ ERREUR : " << nickname << " n'a pas été ajouté comme opérateur !" << std::endl;

			std::ostringstream modeMessage;
			modeMessage << ":" << getNickname(params.client_fd) << "!user@irc.server MODE " 
						<< params.channelName << " +o " << nickname << "\r\n";

			channel->broadcast(modeMessage.str());
		}
		else if (mode == "-o")  
		{
			if (params.Arg.size() < 2)  
			{
				sendError(params.client_fd, "461", "MODE", "Nickname required for -o");
				return;
			}

			const std::string& nickname = params.Arg[1];  // 🔥 Extraction correcte du nickname
			Client* targetClient = getClientByNickname(nickname);
			
			if (!targetClient)
			{
				sendError(params.client_fd, "401", "MODE", "No such nick/channel");
				return;
			}

			if (!channel->isUserInChannel(nickname))
			{
				sendError(params.client_fd, "441", "MODE", nickname + " is not on " + params.channelName);
				return;
			}

			if (!channel->isOperator(params.client_fd))  
			{
				sendError(params.client_fd, "482", "MODE", "You're not a channel operator");
				return;
			}

			// 🔥 Vérifie si la personne est opérateur avant de la retirer
			if (!channel->isOperator(targetClient->getFd()))
			{
				sendError(params.client_fd, "482", "MODE", nickname + " is not an operator");
				return;
			}

			// ✅ Retrait de l'opérateur
			channel->removeOperator(targetClient->getFd());

			if (!channel->isOperator(targetClient->getFd()))
				std::cout << "[DEBUG] ✅ " << nickname << " a perdu son statut d'opérateur sur " << params.channelName << std::endl;
			else
				std::cout << "[DEBUG] ❌ ERREUR : Impossible de retirer " << nickname << " comme opérateur !" << std::endl;

			std::ostringstream modeMessage;
			modeMessage << ":" << getNickname(params.client_fd) << "!user@irc.server MODE " 
						<< params.channelName << " -o " << nickname << "\r\n";

			std::cout << "[DEBUG] 🔥 Message envoyé à tous : " << modeMessage.str();
			channel->broadcast(modeMessage.str());
			}
		else if (mode == "+t")
		{
			channel->setTopicLock(true);
			std::ostringstream modeMessage;
			modeMessage << ":irc.server MODE " << params.channelName << " +t\r\n";
			channel->broadcast(modeMessage.str());
			std::cout << "[DEBUG] ✅ Seuls les opérateurs peuvent maintenant modifier le TOPIC de " << params.channelName << std::endl;
		}
		else if (mode == "-t")
		{
			channel->setTopicLock(false);
			std::ostringstream modeMessage;
			modeMessage << ":irc.server MODE " << params.channelName << " -t\r\n";
			channel->broadcast(modeMessage.str());
			std::cout << "[DEBUG] 🔓 Tout le monde peut maintenant modifier le TOPIC de " << params.channelName << std::endl;
		}
	}
}

void Server::handleModeCommand(std::istringstream& iss, int client_fd)
{
	std::string channelName;
	iss >> channelName;

	if (channelName == getNickname(client_fd)) 
	{
		std::string response = ":" + getNickname(client_fd) + " MODE " + getNickname(client_fd) + " +i\r\n";
		sendMessage(client_fd, response);  // ✅ Utilisation de sendMessage()
		std::cout << "[DEBUG] Confirmed MODE +i for user " << channelName << std::endl;
		return;
	}


	if (channelName.empty())
	{
		sendError(client_fd, "461", "MODE", "Not enough parameters"); // 461: ERR_NEEDMOREPARAMS
		return;
	}

	Channel* channel = getChannelByName(channelName);

	if (!channel)
	{
		sendError(client_fd, "403", "MODE", "No such channel"); // 403: ERR_NOSUCHCHANNEL
		return;
	}

	if (!channel->isOperator(client_fd))
	{
		sendError(client_fd, "482", "MODE", "You're not channel operator"); // 482: ERR_CHANOPRIVSNEEDED
		return;
	}

	std::vector<std::string> args;
	std::string arg;
	while (iss >> arg)
		args.push_back(arg);

	if (args.empty())
	{
		std::string currentModes = channel->isInviteOnly() ? "+i" : "-i"; 
		if (channel->hasKey()) currentModes += "+k";
		if (channel->getUserLimit() > 0) currentModes += "+l";
		if (channel->isTopicLock()) currentModes += "+t";

		sendModeResponse(client_fd, getNickname(client_fd), channelName, "324", currentModes);
		return;
	}

	CommandParams params;
	params.commandType = CommandParams::MODE;
	params.client_fd = client_fd;
	params.channelName = channelName;
	params.Arg = args;

	handleMode(params);
}


void Server::parseModeOptions(const std::vector<std::string>& args, CommandParams& params)
{
	char modeType = 0;

	for (size_t i = 0; i < args.size(); ++i)
	{
		const std::string& token = args[i];

		if (token[0] == '+' || token[0] == '-')
		{
			modeType = token[0];

			for (size_t j = 1; j < token.size(); ++j)
			{
				char mode = token[j];

				if (modeType == '+')
					params.Arg.push_back("+" + std::string(1, mode));
				else if (modeType == '-')
					params.Arg.push_back("-" + std::string(1, mode));
			}
		}
		else 
		{
			if (!params.Arg.empty())
				params.additionalParams.push_back(token);
			else
				std::cerr << "[ERROR] Invalid MODE arguments: " << token << std::endl;
		}
	}
}