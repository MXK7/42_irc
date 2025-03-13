/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:24:27 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/19 15:26:07 by thlefebv         ###   ########.fr       */
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
		bool is_authenticated;
		std::string hostname;
		
	public:

		// Client();
		~Client();
		Client(int fd, const std::string& name, const std::string& nickname, const std::string& hostname = "unknown");
		std::string getName() const ;
		std::string getUsername() const ;
		std::string getNickname() const ;
		int getFd() const ;

		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setName(const std::string& name);

		bool isAuthenticated() const;
    	void authenticate();
		bool notregistered() const;

		std::string getHostname() const;
	    void setHostname(const std::string &host);
};
