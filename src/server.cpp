#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <stdexcept>
#include <system_error>
#include "tuxnet/string.h"
#include "tuxnet/server.h"
#include "tuxnet/log.h"

namespace tuxnet
{

    // Constructor.
    server::server()
    {
        
    }

    // Destructor
    server::~server()
    {

    }

    // Start listening for connections.
    void server::listen(const socket_addresses& saddrs)
    {
        int domain = AF_INET; 
        int type = SOCK_STREAM;
        int fd = 0;
        int result = 0;

        for (auto it = saddrs.begin(); it != saddrs.end() ; ++it)
        {
            // Look up address.
            // Look up protocol number.
            protoent* protocol = getprotobyname("TCP");
            if (protocol == nullptr)
            {
                log::get()->error("Could not get TCP protocol number.");
                return;
            }
            // Create socket.
            fd = socket(domain, SOCK_STREAM, protocol->p_proto);
            if (fd == -1)
            {
                std::string errstr = "Could not open socket, error ";
                errstr += std::to_string(errno);
                errstr += " (";
                errstr += strerror(errno);
                errstr += ")";
                log::get()->error(errstr);
                return;
            }
            // Bind socket.
            const sockaddr_in saddr = dynamic_cast<const ip4_socket_address*>(&(*it))->get_sockaddr_in();
            result = bind(fd, reinterpret_cast<const sockaddr*>(&saddr), sizeof(saddr));
            if (result == -1)
            {
                log::get()->error("Could not bind socket.");
                return;
            }
            // Listen on socket.
            ::listen(fd, 5); /// @TODO : configurable backlog with net.core.somaxconn as default.
        }
    }
    
}

