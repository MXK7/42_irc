/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Part.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/05 13:43:50 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/20 14:00:52 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

void Server::handlePart(const CommandParams& params) {
    Channel* channel = getChannelByName(params.channelName);
    if (!channel) {
        sendError(params.client_fd, "403", params.channelName, "No such channel");
        return;
    }

    Client* partingClient = getClientByFd(params.client_fd);
    if (!partingClient) {
        std::cerr << "[ERROR] Client partant non trouvé pour le FD " << params.client_fd << std::endl;
        return;
    }

    std::ostringstream partMessage;
    partMessage << ":" << partingClient->getNickname() << "!" << partingClient->getUsername() << "@" << partingClient->getHostname() << " PART " << params.channelName << "\r\n";
    channel->broadcast(partMessage.str());

    channel->removeUser(params.client_fd);
    std::cout << "[DEBUG] Client " << partingClient->getNickname() << " a quitté le canal " << params.channelName << std::endl;
}




