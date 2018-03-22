#ifndef SERVER_H_INCLUDE
#define SERVER_H_INCLUDE

#include "tuxnet/string.h"
#include "tuxnet/socket_address.h"

namespace tuxnet
{
    class server
    {

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
             * @param Array of socket address objects containing ip/port/protocol 
             *        information for which ports the server should listen on.
             */
            virtual void listen(const socket_addresses& saddrs);
    
    };

}

#endif
