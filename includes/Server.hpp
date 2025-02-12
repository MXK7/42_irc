/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 20:16:55 by vmassoli          #+#    #+#             */
/*   Updated: 2025/02/12 09:57:44 by thlefebv         ###   ########.fr       */
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
#include <ctime>
#include <cstdlib>  // atoi
#include <sys/socket.h> //-> for socket()
#include <sys/types.h> //-> for socket()
#include <netinet/in.h> //-> for sockaddr_in
#include <arpa/inet.h> //-> for inet_ntoa(), htons()
#include <fcntl.h> //-> for fcntl()
#include <unistd.h> //-> for close()
#include <vector> //-> for vector
#include <string>
#include <sstream>
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

	struct CommandParams
	{
		enum CommandType { JOIN, INVIT, KICK, MODE, TOPIC, PART} commandType;
		int client_fd;
		int operator_fd;
		std::string channelName;
		std::string nickname;
		std::vector<std::string> Arg;
		std::string topicMessage;
		std::string password;
		std::vector<std::string> additionalParams;
		std::string message;
	};


public:

	static Server* instance;

	Server(int port, const std::string& password);
	~Server();

	int CreateSocket();
	int HandlerConnexion();
	void close_all_clients();
	void handle_signal(int signal);
	static void signal_handler(int signal);  // Fonction statique pour signal()

	/*__________________________________________*/

	void addClient(int client_fd, const std::string &name, const std::string &nickname);
	std::string getName(int client_fd);
	std::string getNickname(int client_fd);

	Channel* getChannelByName(const std::string &name);

	std::string getChannelUserListMessage(const std::string& channelName);

	/*________________________________________*/
	void broadcastToChannels(int client_fd, const std::string& message);

	void parseCommand(const std::string& message, int client_fd);
	void handleConnexionCommands(const std::string& command, std::istringstream& iss, int client_fd);
	void handleOtherCommands(const std::string& command, std::istringstream& iss, int client_fd);


	void handleCommand(const CommandParams &params);
	void handleJoin(const CommandParams &params);
	void handleInvit(const CommandParams &params);
	void handleKick(const CommandParams &params);

	void handleMode(const CommandParams &params);
	void handleTopic(int client_fd, const CommandParams &params);
	void parseModeOptions(const std::vector<std::string>& args, CommandParams& params);
	void handleModeCommand(std::istringstream& iss, int client_fd);
	void sendError(int client_fd, const std::string& errorCode, const std::string& command, const std::string& message);
	std::string cleanMessage(const std::string& message);
	void sendWelcomeMessage(int client_fd);

	void sendModeResponse(int client_fd, const std::string& nickname, const std::string& channelName, const std::string& mode, const std::string& extra);
	std::vector<std::string> split_cmd(const std::string& s);

	Client* getClientByNickname(const std::string& nickname);
	Client* getClientByUsername(const std::string& username);
	Client* getClientByFd(int fd);
	Client* getClientByName(const std::string& name);

	bool notregistered(int client_fd);
	void client_authen(int client_fd, const std::string& message);
	void sendMessage(int client_fd, const std::string& message);
	void set_nickname(const std::string& message, int client_fd);
	void set_username(const std::string& message, int client_fd);
	Channel* findChannel(const std::string& channelName);
	int getClientFdByNickname(const std::string& nickname);

	void handleTopic(const CommandParams& params);

	void handlePart(const CommandParams& params);

};
