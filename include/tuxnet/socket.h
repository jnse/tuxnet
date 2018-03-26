/**
 * Network socket definition.
 **/

#ifndef SOCKET_H_INCLUDE
#define SOCKET_H_INCLUDE

#include <vector>
#include "tuxnet/socket_address.h"
#include "tuxnet/protocol.h"

namespace tuxnet
{

    /**
     * Network socket.
     */
    class socket
    {

        // Private member variables. ------------------------------------------

        /// Stores the local address/port pair.
        socket_address* const m_local_saddr;

        /// Stores the remote address/port pair.
        socket_address* const m_remote_saddr;

        /// Stores the socket protocol.
        layer4_protocol m_proto;

        /// Stores file descriptor for the socket.
        int m_fd;

        // Private member functions. ------------------------------------------

        /// Binds the socket to an ipv4 address.
        bool m_ip4_bind();

        /// Binds the socket to an ipv6 address.
        /// \@TODO add ipv6 support.
        bool m_ip6_bind();

        public:

            // Constructors. --------------------------------------------------

            /** 
             * Constructor with local/remote saddrs.
             * @param proto : Protocol to be used for the socket.
             */
            socket(const layer4_protocol& proto);

            // Getters. -------------------------------------------------------

            /**
             * Gets ip/port information for local side of the connection.
             * @return Returns socket_address object containing ip/port 
             *         information for local end of the connection.
             */
            const socket_address& get_local() const;

            /**
             * Gets ip/port information for remote side of the connection.
             * @return Returns socket_address object containing ip/port 
             *         information for remote end of the connection.
             */
            const socket_address& get_remote() const;

            /**
             * Gets the protocol used for this socket.
             * @return Returns the protocol used for this socket.
             */
            const layer4_protocol get_proto() const;
           
            // Methods. -------------------------------------------------------

            /**
             * Binds the socket to an address/port pair.
             *
             * This function is automatically called by listen(), and 
             * connect(), so you shouldn't have to invoke it manually.
             *
             * @param saddr : socket_address object containing address/port.
             * @return Returns true on success, false on failure.
             */
            bool bind(const socket_address& saddr);

            /**
             * Listens on an address/port pair.
             *
             * @param saddr : socket_address object containing address/port.
             * @return Returns true on success, false on failure.
             */
            bool listen(const socket_address& saddr);

    };

    /// Collection of sockets.
    typedef std::vector<tuxnet::socket*> sockets;

}

#endif

