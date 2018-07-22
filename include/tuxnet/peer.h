#ifndef PEER_H_INCLUDE
#define PEER_H_INCLUDE

#include <netinet/in.h>
#include <unordered_map>
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

            // Getters. -------------------------------------------------------

            /**
             * Get file descriptor.
             * @return Returns file descriptor associated with this peer 
             *         connection.
             */
            int get_fd();

            /**
             * Get socket address.
             * @return Returns a pointer the socket_address object containing 
             *         the address and port pair for the peer.
             */
            socket_address* get_saddr();

            // Methods. -------------------------------------------------------

            /**
             * Reads up to a given number of characters from the 
             * peer socket.
             */
            std::string read_string(int characters);

            /**
             * Reads string until token is sent.
             */
            std::string read_string_until(std::string token);

            /**
             * Reads a line of text.
             */
            std::string read_line();

    };

    /// Collection of peers.
    typedef std::unordered_map<int, peer*> peers;

}

#endif
