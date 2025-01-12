/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vmassoli <vmassoli@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/11 13:24:27 by thlefebv          #+#    #+#             */
/*   Updated: 2025/01/11 20:44:08 by vmassoli         ###   ########.fr       */
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

	public:

		// Client();
		~Client();
		Client(int fd, const std::string& name, const std::string& nickname);

		std::string getName() const ;
		std::string getNickname() const ;
		int getFd() const ;
};
