/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmassoli <vmassoli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 20:16:55 by vmassoli          #+#    #+#             */
/*   Updated: 2025/01/12 12:35:38 by vmassoli         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Client.hpp"
#include "Channel.hpp"

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN "\033[36m"
#define COLOR_WHITE "\033[37m"


#include <iostream>
#include <cstring>
#include <cerrno>
#include <cstdlib>  // atoi
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <arpa/inet.h> //-> for inet_ntoa(), htons()
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <vector> //-> for vector
#include <string>
#include <csignal> //-> for signal()
#include <algorithm>
// #include <poll.h> //-> for poll()


#define ERR_CR_SOCK "Error : Failed to create a socket."
#define ERR_CFG_SOCK "Error : Cannot configure socket options."
#define ERR_BIND_SOCK  "Error: failed to bind the socket."
#define ERR_LISTEN_SOCK "Error: failed to listen to the socket."
#define ERR_FCNTL_SOCK "Error: Unable to make socket non-blocking"

class Server
{
private:
	std::vector<Channel*> channels;
	std::vector<Client*> clients;
	int server_fd;
	std::vector<int> client_fds;
	bool is_running;
	int port;
	std::string password;

public:

	static Server* instance;

	Server(int port, const std::string& password);
	~Server();

	int CreateSocket();
	int HandlerConnexion();
	void close_all_clients();
	void handle_signal(int signal);
	static void signal_handler(int signal);  // Fonction statique pour signal()

	/*________________________________________*/

	void handleCommand();

	void handleJoin(int client_fd, const std::string &channelName, const std::string &nickname);
	void handleInvit(int operator_fd, const std::string &channelName, const std::string &nickname);
	// void handleKick(int client_fd, const std::string &channelName);
	// void handleMode(int client_fd, const std::string &channelName);
	// void handleTopic(int client_fd, const std::string &channelName);

	/*__________________________________________*/
	void addClient(int client_fd, const std::string &name, const std::string &nickname);
	std::string getName(int client_fd);
	std::string getNickname(int client_fd);

};
