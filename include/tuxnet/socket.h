/**
 * Network socket definition.
 **/

#ifndef SOCKET_H_INCLUDE
#define SOCKET_H_INCLUDE

#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include "tuxnet/socket_address.h"
#include "tuxnet/protocol.h"
#include "tuxnet/peer.h"

namespace tuxnet
{

    // Forward declaration for tuxnet::server
    class server;

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
     *
     * @TODO Implement a keepalive mechanism to detect closed connections.
     *       (send 0 bytes periodically at a configurable keepalive interval).
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

        /// Storage for peer connections.
        peers m_peers;

        // Private member functions. ------------------------------------------

        /// Binds the socket to an ipv4 address.
        bool m_ip4_bind();

        /// Binds the socket to an ipv6 address.
        /// @TODO add ipv6 support.
        bool m_ip6_bind();

        /// Attempts to accept in incomming connection.
        /// @return Returns true on success, false otherwise.
        peer* m_try_accept();

        /**
         * Attempts to make a file-descriptor non-blocking.
         * @return Returns true on success, false otherwise.
         */
        bool m_make_fd_nonblocking(int fd);

        /**
         * Adds a file-descriptor to epoll monitoring.
         * @return Returns true on  success, false otherwise.
         */
        bool m_monitor_fd(int fd);

        /**
         * Clean up after peer(s) with given file descriptor.
         * @param fd File-descriptor of peer to reap.
         */
        void m_remove_peer(int fd);

        /**
         * Pointer to server object owning this socket.
         * in case we're a listener socket.
         */
        server* m_server;

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

            /// Closes the socket.
            void close();

            /**
             * Listens on an address/port pair.
             *
             * @param saddr : socket_address object containing address/port.
             * @param server : (optional) pointer to tuxnet::server instance 
             *                 owning this socket if called from a 
             *                 server object.
             * @return Returns true on success, false on failure.
             */
            bool listen(const socket_address* saddr, 
                server* server_object=nullptr);

            /**
             * Checks if any events happened on the socket.
             * @return Returns true on success, false on error.
             */
            bool poll();

    };

    /// Collection of sockets.
    typedef std::vector<tuxnet::socket*> sockets;

}

#endif

