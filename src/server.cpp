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
        log::debug("server::listen"); 
        return;
        unsigned short port = 80;
        char* ip_address = nullptr;
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

            }

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
                throw std::system_error(2, std::generic_category(), errstr); 
            }

            /// \TODO ipv6 support.
            /// \TODO UDP support.
            // Look up protocol number.
            protoent* protocol = getprotobyname("TCP");
            if (protocol == nullptr)
            {
                throw std::system_error(1, std::generic_category(),
                    "Could not get TCP protocol number.");
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
                throw std::system_error(2, std::generic_category(), errstr);
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
        }
    }
    
}

