#ifndef SERVER_H_INCLUDE
#define SERVER_H_INCLUDE

#include "tuxnet/string.h"
#include "tuxnet/socket_address.h"
#include "tuxnet/socket.h"

namespace tuxnet
{
    class server
    {

        sockets listen_sockets;

        public:

            /**
             * Constructor.
             */
            server();

            /**
             * Destructor.
             */
            virtual ~server();

            /**
             * Start listening for connections.
             *
             * @param saddrs : Array of socket address objects containing
             *        ip/port/protocol information for which ports the server
             *        should listen on.
             * @param proto : Layer-4 protocol the server should listen on.
             */
            virtual void listen(const socket_addresses& saddrs, 
                const layer4_protocol& proto);
    
    };

}

#endif
