/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: thlefebv <thlefebv@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 15:07:18 by thlefebv          #+#    #+#             */
/*   Updated: 2025/02/26 13:59:48 by thlefebv         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Server.hpp"

int Server::checkPassword(int client_fd, const std::string& receivedPassword)
{
    // std::cout << " Mot de passe reçu : " << receivedPassword << std::endl;
    // std::cout << " Mot de passe attendu : " << this->password << std::endl;

    if (receivedPassword != this->password)
    {
        sendError(client_fd, "464", "PASS", "Password incorrect"); // 464: ERR_PASSWDMISMATCH
        std::cerr << COLOR_RED << "[SECURITY] Mot de passe incorrect pour FD: " << client_fd << COLOR_RESET << std::endl;

        // **FERMETURE PROPRE (Ne pas utiliser FD_CLR ici)**
        close(client_fd);

        return 1; // ❌ Mauvais mot de passe
    }

    // std::cout << " Client FD " << client_fd << " authentifié avec password.\n";
    sendMessage(client_fd, ":server NOTICE * :Password accepted, proceed with NICK and USER\r\n");
    return 0; // ✅ Mot de passe valide
}

