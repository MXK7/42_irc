/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:24:27 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/22 15:17:32 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// #include "Server.hpp"
// #include "Channel.hpp"
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

	public:

		// Client();
		~Client();
		Client(int fd, const std::string& name, const std::string& nickname);

		std::string getName() const ;
		std::string getUsername() const ;
		std::string getNickname() const ;
		int getFd() const ;

		void setNickname(const std::string& nickname);
		void setUsername(const std::string& username);
		void setName(const std::string& name);

		bool isAuthenticated() const { return is_authenticated; }
    	void authenticate() { is_authenticated = true; }

};
