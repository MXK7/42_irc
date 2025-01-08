#include "Server.hpp"
/**
 * @brief Create a server socket

This function initializes a server socket using the IPv4 protocol (AF_INET) and TCP (SOCK_STREAM). 
It also sets the `SO_REUSEADDR` option, which allows the server to reuse the address without waiting 
for the operating system to release the port.

 * @return true if the socket was created and configured successfully, false otherwise.

 * @socket: 
    - AF_INET = IPv4
    - SOCK_STREAM = Reliable and ordered communication (TCP)
    - 0 = Automatically selects the appropriate protocol (TCP for SOCK_STREAM, UDP for SOCK_DGRAM)

 * @setsockopt: 
    - tmp_opt = 1: Enables the reuse option.
    - SOL_SOCKET, SO_REUSEADDR: Allows the address to be reused.

 * @sockaddr_in:
    struct sockaddr_in {
        sa_family_t    sin_family;   // AF_INET (IPv4)
        uint16_t       sin_port;     // Port (big-endian grâce à htons)
        struct in_addr sin_addr;     // Adresse IP (ici INADDR_ANY)
        char           sin_zero[8];  // Non utilisé (rempli par memset)
    };

 * @INADDR_ANY:
    Authorize all connections on all local interfaces.

 * @errors: 
    - Prints ERR_CR_SOCK if socket creation fails.
    - Prints ERR_CFG_SOCK and exits the program if configuration fails.
 */
int Server::CreateSocket()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        std::cerr << COLOR_RED << ERR_CR_SOCK << std::endl;
        return (-1);
    }

    int tmp_opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &tmp_opt, sizeof(tmp_opt)) < 0)
    {
        std::cerr << COLOR_RED << ERR_CFG_SOCK << std::endl;
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    // server_addr.sin_port = htons(port);

    return (0);
}
