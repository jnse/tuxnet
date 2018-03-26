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
    void server::listen(const socket_addresses& saddrs, const layer4_protocol& proto)
    {
        int domain = AF_INET; 
        int type = SOCK_STREAM;
        int fd = 0;
        int result = 0;

        for (auto it = saddrs.begin(); it != saddrs.end() ; ++it)
        {
            // Create socket and start listening.
            socket* sock = new socket(proto);
            // Listen on socket.
            sock->listen((*it));
        }
    }
    
}

