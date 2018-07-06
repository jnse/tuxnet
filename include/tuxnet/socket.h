/**
 * Network socket definition.
 **/

#ifndef SOCKET_H_INCLUDE
#define SOCKET_H_INCLUDE

#include <sys/epoll.h>
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
        const socket_address* m_local_saddr;

        /// Stores the remote address/port pair.
        const socket_address* m_remote_saddr;

        /// Stores the socket protocol.
        layer4_protocol m_proto;

        /// Stores file descriptor for the socket.
        int m_fd;

        /// Stores epoll file descriptor for polling the socket.
        int m_epoll_fd;

        /// epoll event buffer.
        epoll_event* m_epoll_events;

        // Private member functions. ------------------------------------------

        /// Binds the socket to an ipv4 address.
        bool m_ip4_bind();

        /// Binds the socket to an ipv6 address.
        /// \@TODO add ipv6 support.
        bool m_ip6_bind();

        public:

            // ctor(s) / dtor. ------------------------------------------------

            /** 
             * Constructor with local/remote saddrs.
             * @param proto : Protocol to be used for the socket.
             */
            socket(const layer4_protocol& proto);

            /// Destructor.
            ~socket();

            // Getters. -------------------------------------------------------

            /**
             * Gets ip/port information for local side of the connection.
             * @return Returns socket_address object containing ip/port 
             *         information for local end of the connection.
             */
            const socket_address* get_local() const;

            /**
             * Gets ip/port information for remote side of the connection.
             * @return Returns socket_address object containing ip/port 
             *         information for remote end of the connection.
             */
            const socket_address* get_remote() const;

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
            bool bind(const socket_address* saddr);

            /**
             * Listens on an address/port pair.
             *
             * @param saddr : socket_address object containing address/port.
             * @return Returns true on success, false on failure.
             */
            bool listen(const socket_address* saddr);

            /**
             * Checks if any events happened on the socket.
             */
            void poll();

    };

    /// Collection of sockets.
    typedef std::vector<tuxnet::socket*> sockets;

}

#endif

