#ifndef PEER_H_INCLUDE
#define PEER_H_INCLUDE

#include <netinet/in.h>
#include "tuxnet/socket_address.h"

namespace tuxnet
{

    /**
     * Peer.
     *
     * This object represents a remote peer.
     */
    class peer
    {
        /// File descriptor.
        int m_fd;
        /// IP and port of peer.
        socket_address* m_saddr;

        public:

            // ctor(s) / dtor. ------------------------------------------------

            /**
             * IPV4 constructor.
             *
             * @param fd : File descriptor associated with connection.
             * @param in_addr : sockaddr for peer connection.
             */
            peer(int fd, const sockaddr_in& in_addr);

            /**
             * IPV6 constructor.
             *
             * @param fd : File descriptor associated with connection.
             * @param in_addr : sockaddr for peer connection.
             */
            peer(int fd, const sockaddr_in6& in_addr);

            /// Destructor.
            ~peer();

    };


}

#endif
