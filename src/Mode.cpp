/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:25:46 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/05 10:12:06 by thlefebv         ###   ########.fr       */
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
            if (++argIt != params.Arg.end())
            {
                int limit = std::atoi(argIt->c_str());
                channel->setUserLimit(limit);
                std::ostringstream oss;
                oss << limit;
                sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "+l", oss.str());
            }
            else
            {
                sendError(params.client_fd, "461", "MODE", "Limit is missing");
                return;
            }
        }
        else if (mode == "-l")
        {
            channel->clearUserLimit();
            sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "-l", "");
        }
        else
        {
            sendError(params.client_fd, "472", mode, "Unknown mode character");
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