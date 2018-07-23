/**
 * Network socket definition.
 **/

#ifndef SOCKET_H_INCLUDE
#define SOCKET_H_INCLUDE

#include <atomic>
#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include "tuxnet/socket_address.h"
#include "tuxnet/protocol.h"
#include "tuxnet/lockable.h"
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
     * @todo Implement a keepalive mechanism to detect closed connections.
     *       (send 0 bytes periodically at a configurable keepalive interval).
     */
    class socket
    {

        friend class server;

        // Private member variables. ------------------------------------------

        /// epoll event buffer.
        epoll_event* m_epoll_events;

        /// Stores epoll file descriptor for polling the socket.
        int m_epoll_fd;

        /// epoll max_events (epoll event buffer size)
        int m_epoll_maxevents;

        /// Stores file descriptor for the socket.
        std::atomic<int> m_fd;

        /// Use (or don't) keepalive for this socket.
        bool m_keepalive;

        /// Keepalive packet interval (in seconds).
        int m_keepalive_interval;

        /// Keepalive retries
        int m_keepalive_retry;

        /// Keepalive timeout (in seconds).
        int m_keepalive_timeout;

        /// Stores the local address/port pair.
        const socket_address* m_local_saddr;

        /// Stores the socket protocol.
        layer4_protocol m_proto;

        /// Stores the remote address/port pair.
        const socket_address* m_remote_saddr;

        /**
         * Pointer to server object owning this socket.
         * in case we're a listener socket.
         */
        server* m_server;

        /// Stores the current state of the socket.
        socket_state m_state;

        // Private member functions. ------------------------------------------

        /**
         * @brief Enables keepalive on the socket if m_keepalive is true.
         * @param fd : File descriptor to enable keepalive for.
         * @return Returns true on success, false on error.
         */
        bool m_enable_keepalive(int fd);

        /// Binds the socket to an ipv4 address.
        bool m_ip4_bind();

        /// Binds the socket to an ipv6 address.
        /// @todo add ipv6 support.
        bool m_ip6_bind();

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

        /// Attempts to accept in incomming connection.
        /// @return Returns true on success, false otherwise.
        peer* m_try_accept();

        // Protected member variables. ----------------------------------------

        protected:

            /// Storage for peer connections.
            lockable<peers> m_peers;


        public:

            // ctor(s) / dtor. ------------------------------------------------

            /** 
             * @brief Constructor with local/remote saddrs.
             * @param proto : Protocol to be used for the socket.
             * @param epoll_max_events : (optional) Size of epoll event buffer.
             */
            socket(const layer4_protocol& proto, int epoll_max_events=64);

            /// Destructor.
            ~socket();

            // Getters / setters. ----------------------------------------------

            /**
             * @brief Gets whether or not keepalive is enabled for this socket.
             * 
             * See tuxnet::socket::set_keepalive() for more
             * details on the keepalive mechanism.
             *
             * @return Returns true if keepalive is enabled, false otherwise.
             */
            bool get_keepalive() const;

            /**
             * @brief Returns the keepalive interval for this socket.
             *
             * See tuxnet::socket::set_keepalive() for more
             * details on the keepalive mechanism.
             *
             * @return Returns the keepalive interval for this socket
             *         (in seconds).
             */
            int get_keepalive_interval() const;

            /**
             * @brief Returns the keepalive retries for this socket.
             *
             * See tuxnet::socket::set_keepalive() for more
             * details on the keepalive mechanism.
             *
             * @return Returns the number of TCP keepalive probes are to be
             *         sent before considering the remote peer to be dead.
             */
            int get_keepalive_retry() const;

            /**
             * @brief Returns the keepalive timeout for this socket.
             *
             * See tuxnet::socket::set_keepalive() for more
             * details on the keepalive mechanism.
             */
            int get_keepalive_timeout() const;

            /**
             * @brief Gets ip/port information for local side of the 
             *        connection.
             * @return Returns socket_address object containing ip/port 
             *         information for local end of the connection.
             */
            const socket_address* const get_local() const;

            /**
             * @brief Gets the protocol used for this socket.
             * @return Returns the protocol used for this socket.
             */
            layer4_protocol get_proto() const;

            /**
             * @brief Gets ip/port information for remote side of the 
             *        connection.
             * @return Returns socket_address object containing ip/port 
             *         information for remote end of the connection.
             */
            const socket_address* const get_remote() const;

            /**
             * @brief Sets whether or not keepalive should be enabled for this 
             *        socket.
             *
             * @note This is only relevant for TCP sockets.
             *
             * The keepalive mechanism is used for detecting dropped 
             * connections for when peers don't (or can't) gracefully
             * disconnect, thus preventing a half-open state.
             *
             * The way this works is by sending 0-byte packets periodically 
             * to ping the connection. Since these packets still need to be
             * ACK'ed, we can poll and disconnect the socket if there is no
             * reply.
             *
             * Because this mechanism takes advantage of the TCP
             * guaranteed-delivery mechanism (every packet needs to be
             * acknowledged), on a non-TCP socket the keepalive mechanism 
             * is not used, and calls to tuxnet::socket::set_keepalive(),
             * tuxnet::socket::set_keepalive_timeout(), and
             * tuxnet::socket::set_keepalive_interval() are a no-op.
             *
             * The keepalive mechanism is on by default, so you don't have to
             * call tuxnet::socket::set_keepalive unless you explicitly want
             * to disable it (or re-enable it after disabling it).
             *
             * Underneath the hood, the TCP keepalive mechanism is handled 
             * entirely by the kernel, and is enabled by setting options on
             * the socket's file descriptor using setsockopt().
             *
             * These functions correspond to the following
             * setsockopt options:
             *
             * | tuxnet function          | setsockopt option          |
             * | ------------------------ | -------------------------- |
             * | set_keepalive()          | SOL_SOCKET, SO_KEEPALIVE   |
             * | set_keepalive_interval() | IPPROTO_TCP, TCP_KEEPINTVL |
             * | set_keepalive_timeout()  | IPPROTO_TCP, TCP_KEEPIDLE  |
             * | set_keepalive_retry()    | IPPROTO_TCP, TCP_KEEPCNT   |
             *
             * These are described in 
             * [man (7) TCP](http://man7.org/linux/man-pages/man7/socket.7.html)
             *
             * @param keepalive_enabled : Set this to false to disable 
             *                            the keepalive mechanism, or set it to
             *                            true to enable it.
             */
            void set_keepalive(bool keepalive_enabled);
            
            /**
             * @brief Sets the keepalive interval.
             *
             * @note This is only relevant for TCP sockets.
             *
             * Governs how frequently we send TCP keepalive packets.
             *
             * Default is every 5 seconds.
             *
             * See tuxnet::socket::set_keepalive() for more information on the
             * TCP keepalive system.
             *
             * @param interval : TCP keepalive interval in seconds.
             */
            void set_keepalive_interval(int interval);

            /**
             * @brief Sets the keepalive retries.
             *
             * @note This is only relevant for TCP sockets.
             *
             * Governs how many TCP keepalive probes should be sent until the
             * connection can be considered dropped.
             *
             * See tuxnet::socket::set_keepalive() for more information on the
             * TCP keepalive system.
             *
             * The default is 3 probes before the connection is considered
             * dead.
             *
             * @param retries : Number of TCP keepalive retries before giving
             *                  up on the remote peer.
             */
            void set_keepalive_retry(int retries);

            /**
             * @brief Sets the keepalive timeout.
             *
             * @note This is only relevant for TCP sockets.
             *
             * This governs how long a connection should be idle before
             * the kernel should start sending keepalive probes.
             *
             * The default is 10 seconds.
             *
             * See tuxnet::socket::set_keepalive() for more information on the
             * TCP keepalive system.
             *
             * @param timeout : Sets the TCP keepalive timeout (in seconds).
             */
            void set_keepalive_timeout(int timeout);

            // Methods. -------------------------------------------------------

            /**
             * @brief Binds the socket to an address/port pair.
             *
             * This function is automatically called by listen(), and 
             * connect(), so you shouldn't have to invoke it manually.
             *
             * @param saddr : socket_address object containing address/port.
             * @return Returns true on success, false on failure.
             */
            bool bind(const socket_address* const saddr);

            /// Closes the socket.
            void close();

            /**
             * Disconnects a remote peer.
             *
             * @param client : Pointer to peer to disconnect.
             */
            void disconnect(peer* client);

            /**
             * @brief Listens on an address/port pair.
             *
             * @param saddr : socket_address object containing address/port.
             * @param server_object : (optional) pointer to tuxnet::server
             *                 instance owning this socket if called from a 
             *                 server object.
             * @return Returns true on success, false on failure.
             */
            bool listen(const socket_address* const saddr, 
                server* server_object=nullptr);

            /**
             * @brief Checks if any events happened on the socket.
             * @return Returns true on success, false on error.
             */
            bool poll();

    };

    /// Collection of sockets.
    typedef std::vector<tuxnet::socket*> sockets;

}

#endif

