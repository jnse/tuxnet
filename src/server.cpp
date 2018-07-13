#include <iostream>
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
    bool server::listen(const socket_addresses& saddrs, const layer4_protocol& proto)
    {
        int domain = AF_INET; 
        int type = SOCK_STREAM;
        int fd = 0;
        int result = 0;
        bool err = false;
        for (auto it = saddrs.begin(); it != saddrs.end() ; ++it)
        {
            // Create socket and start listening.
            socket* sock = new socket(proto);
            // Listen on socket.
            if (sock->listen(*it, this) != true) err = true;
            /// @TODO remove test loop below.
            while(true)
            {
                sock->poll();
            }
        }
        return !err;
    }

    // Events. ----------------------------------------------------------------

    void server::on_connect(peer* remote_peer)
    {
        log::get()->debug("Received connection.");
    }

}

