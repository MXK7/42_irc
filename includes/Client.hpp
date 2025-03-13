/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rcarbonn <rcarbonn@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:24:27 by thlefebv          #+#    #+#             */
/*   Updated: 2025/03/13 15:12:21 by rcarbonn         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>


class Client
{
	private:

		int fd;
		std::string name;
		std::string nickname;
		std::string username;
		std::string password;
		bool is_authenticated;
		std::string hostname;
		
	public:

		// Client();
		~Client();
		Client(int fd, const std::string& name, const std::string& nickname, const std::string& hostname = "unknown");
		std::string getName() const ;
		std::string getUsername() const ;
		std::string getNickname() const ;
		std::string getPassword() const ;
		int getFd() const ;

		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setPassword(const std::string& password);
		void setName(const std::string& name);

		bool isAuthenticated() const;
    	void authenticate();
		bool notregistered() const;

		std::string getHostname() const;
	    void setHostname(const std::string &host);
};
