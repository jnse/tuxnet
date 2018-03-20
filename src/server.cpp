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
    void server::listen(str_vector listen_addresses)
    {
        unsigned short port = 80;
        const char* ip_address = nullptr;
        int domain = AF_INET; 
        int type = SOCK_STREAM;
        int fd = 0;
        int result = 0;

        for (auto it = listen_addresses.begin();
            it != listen_addresses.end() ; ++it)
        {
            // Parse IP and port.
            str_vector ip_and_port = str_split((*it),":",2);
            if (ip_and_port.size() != 2)
            {
                log::get()->error("Invalid IP/port pair specified: "+(*it));
                return;
            }
            ip_address = ip_and_port[0].c_str();
            port = std::stoi(ip_and_port[1]);
            // Look up address.
            in_addr ip_address_inaddr = {};
            result = inet_aton(ip_address, &ip_address_inaddr);
            if (result != 1)
            {
                std::string errstr = "The address ";
                errstr += ip_address;
                errstr += " is invalid, error ";
                errstr += std::to_string(errno);
                errstr += " (";
                errstr += strerror(errno);
                errstr += ")";
                log::get()->error(errstr);
                return;
            }
            /// \TODO ipv6 support.
            /// \TODO UDP support.
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
            // Create a sockaddr structure representing the bind address.
            sockaddr_in bind_address = {};
            bind_address.sin_family = domain;
            bind_address.sin_port = htons(port);
            bind_address.sin_addr.s_addr = inet_netof(ip_address_inaddr);
            // Bind socket.
            result = bind(fd, 
                const_cast<const sockaddr*>(
                    reinterpret_cast<sockaddr*>(&bind_address)
                ), 
                sizeof(bind_address)
            );
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

