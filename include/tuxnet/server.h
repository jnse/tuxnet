#ifndef SERVER_H_INCLUDE
#define SERVER_H_INCLUDE

#include "tuxnet/string.h"
#include "tuxnet/socket_address.h"
#include "tuxnet/peer.h"
#include "tuxnet/socket.h"

namespace tuxnet
{
    class server
    {

        friend class socket;
        /// Listening sockets.
        sockets listen_sockets;

        public:

            // Ctor(s) / dtor. ------------------------------------------------

            /**
             * Constructor.
             */
            server();

            /**
             * Destructor.
             */
            virtual ~server();

            // Methods. -------------------------------------------------------

            /**
             * Start listening for connections.
             *
             * @param saddrs : Array of socket address objects containing
             *        ip/port/protocol information for which ports the server
             *        should listen on.
             * @param proto : Layer-4 protocol the server should listen on.
             */
            virtual bool listen(const socket_addresses& saddrs, 
                const layer4_protocol& proto);

        protected:

            // Events. --------------------------------------------------------

            virtual void on_connect(peer* remote_peer);

    };

}

#endif
