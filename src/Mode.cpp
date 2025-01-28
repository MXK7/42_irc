/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 10:25:46 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/28 14:11:25 by thlefebv         ###   ########.fr       */
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
                const std::string& key = *argIt;
                channel->setKey(key);
                sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "+k", key);
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
                std::string errorMsg = ":" + getNickname(params.client_fd) +
                                       " 461 " + params.channelName + " +l :Limit is missing\r\n";
                send(params.client_fd, errorMsg.c_str(), errorMsg.size(), 0);
                return;
            }
        }
        else if (mode == "-l")
        {
            channel->clearUserLimit();
            sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "-l", "");
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
                    sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "+o", nickname);
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
                    sendModeResponse(params.client_fd, getNickname(params.client_fd), params.channelName, "-o", nickname);
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

    if (!channel)
    {
        std::string errorMsg = ":" + getNickname(client_fd) +
                               " 403 " + channelName + " :No such channel\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
        return;
    }

    if (!channel->isOperator(client_fd))
    {
        std::string errorMsg = ":" + getNickname(client_fd) +
                               " 482 " + channelName + " :You're not channel operator\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.size(), 0);
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
