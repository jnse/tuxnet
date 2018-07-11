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

    /// Enum for the different states a socket can be in.
    enum socket_state
    {
        /// SOCKET_STATE_UNINITIALIZED is the default initial state.
        SOCKET_STATE_UNINITIALIZED=0,
        /**
         * SOCKET_STATE_LISTENING is for tcp server sockets in the 
         * listening state.
         **/
        SOCKET_STATE_LISTENING,
        /** 
         * SOCKET_STATE_CONNECTING is for tcp client sockets that have not yet
         *                         completed the tcp 3-way handshake after
         *                         initiating a connection attempt with
         *                         a server.
         **/
        SOCKET_STATE_CONNECTING,
        /**
         * SOCKET_STATE_CONNECTED is used for TCP client sockets which have 
         *                        completed the 3-way handshake with a server.
         **/
        SOCKET_STATE_CONNECTED,
        /**
         * SOCKET_STATE_STATELESS is used for conectionless protocols 
         *                        like UDP.
         **/
        SOCKET_STATE_STATELESS,
        /// SOCKET_STATE_CLOSING is used for a socket that is shutting down.
        SOCKET_STATE_CLOSING,
        /**
         * SOCKET_STATE_CLOSED is used for a socket that completed 
         *                     shutting down.
         **/
        SOCKET_STATE_CLOSED
    };

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

        /// epoll max_events (epoll event buffer size)
        int m_epoll_maxevents;

        /// Stores the current state of the socket.
        socket_state m_state;

        // Private member functions. ------------------------------------------

        /// Binds the socket to an ipv4 address.
        bool m_ip4_bind();

        /// Binds the socket to an ipv6 address.
        /// \@TODO add ipv6 support.
        bool m_ip6_bind();

        /// Attempts to accept in incomming connection.
        /// \@return Returns true on success, false otherwise.
        bool m_try_accept();

        public:

            // ctor(s) / dtor. ------------------------------------------------

            /** 
             * Constructor with local/remote saddrs.
             * @param proto : Protocol to be used for the socket.
             * @param epoll_max_events : (optional) Size of epoll event buffer.
             */
            socket(const layer4_protocol& proto, int epoll_max_events=64);

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

            // Events. --------------------------------------------------------

            /**
             * Connect event.
             *
             * This event fires when a listening socket receives a client
             * connection. Typically you would either deny (or ignore) the
             * connection attempt here, or accept the connection from the
             * client by calling accept().
             *
             * @param peer IP and port of the client that initiated the
             *             connection attempt.
             */
            void on_connect(const socket_address* peer);

    };

    /// Collection of sockets.
    typedef std::vector<tuxnet::socket*> sockets;

}

#endif

