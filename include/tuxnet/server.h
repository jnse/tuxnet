#ifndef SERVER_H_INCLUDE
#define SERVER_H_INCLUDE

#include <future>
#include "tuxnet/string.h"
#include "tuxnet/socket_address.h"
#include "tuxnet/peer.h"
#include "tuxnet/lockable.h"
#include "tuxnet/socket.h"

namespace tuxnet
{
    class server
    {

        friend class socket;

        // Private member variables. ------------------------------------------

        /// Keepalive enabled?
        bool m_keepalive;
        /// Keepalive interval.
        int m_keepalive_interval;
        /// Keepalive retries.
        int m_keepalive_retries;
        /// Keepalive timeout.
        int m_keepalive_timeout;
        /// Listening sockets.
        lockable<sockets> m_listen_sockets;

        // Private member functions. ------------------------------------------

        /** 
         * Polls a single socket.
         *
         * Calls poll on the given socket asynchronously.
         *
         * @param sock Socket to call poll() on.
         * @return Returns a future which returns true on success, 
         *         false on failure
         */
        static bool m_poll_single(socket* const sock);

        public:

            // Ctor(s) / dtor. ------------------------------------------------

            /// Constructor.
            server();

            /// Destructor.
            virtual ~server();

            // Methods. -------------------------------------------------------

            /**
             * @brief Configures TCP keepalive settings.
             *
             * @note Only relevant for TCP servers.
             *
             * Allows you to tune the TCP keepalive timeout, interval, 
             * and retry settings that will be used for client connections.
             *
             * These settings govern detection of broken connections and 
             * half-open states by pinging the other end with 0-byte packets
             * which need to be ACK'd by the peer.
             *
             * A lot more details on this can be found under the documentation
             * for tuxnet::socket::set_keepalive()
             *
             * @param enabled : Set to false to disable, or true to enable.
             *                  (it's enabled by default, so you don't have
             *                  to call this to enable keepalive).
             * @param timeout : (optional) How long a client can be idle until
             *                  we should start sending keepalive probes.
             * @param interval : (optional) Delay between keepalive probes.
             * @param retries : (optional) How many times a probe can fail 
             *                  (no ack received from client) until we consider
             *                  the client dead and disconnected.
             */
            void configure_keepalive(bool enabled, int timeout=10, 
                int interval=5, int retries=3);

            /**
             * @brief Start listening for connections.
             *
             * @param saddrs : Array of socket address objects containing
             *        ip/port/protocol information for which ports the server
             *        should listen on.
             * @param proto : Layer-4 protocol the server should listen on.
             */
            virtual bool listen(const socket_addresses& saddrs, 
                const layer4_protocol& proto);

            /**
             * Returns number of connected clients.
             * @return Number of connected clients.
             */
            int num_clients();

            /**
             * @brief Poll the server to process events.
             *
             * Processes events for incomming connections and/or data.
             *
             * @return Returns true on success, false on error.
             */
            bool poll();

        protected:

            // Events. --------------------------------------------------------

            /**
             * @brief on_connect event.
             *
             * Override this method in order to handle connecting clients.
             *
             * This event fires whenever a client connects.
             *
             * A pointer to a peer object is passed which allows you to get
             * information on (and interact with) the client.
             * 
             * If you'd like to deny connections from a client, this would be a
             * good place to do it.
             *
             * @param remote_peer : peer object representing the connecting 
             *                      client.
             */
            virtual void on_connect(peer* remote_peer);

            /**
             * @brief on_receive event.
             *
             * Override this method in order to handle incomming data from a
             * client.
             *
             * This event fires whenever a client sends data.
             *
             * A pointer to a peer object is passed which allows you to get
             * information on (and interact with) the client.
             * 
             * This is where you'd typically call remote_peer->read() or 
             * similar functions to read data back from the client.
             * 
             * @param remote_peer : peer object representing the client which
             *                      sent the data for which the event was
             *                      triggered.
             */
            virtual void on_receive(peer* remote_peer);

            /**
             * @brief on_disconnect event.
             *
             * Override this method in order to handle disconnecting clients.
             *
             * This event fires whenever a client disconnects gracefully, or
             * when a client connection times out (if TCP keepalive is 
             * enabled anyway - it is by default. See configure_keepalive() for
             * details on TCP keepalive).
             *
             * This is where you would typically place cleanup code for
             * disconnecting clients for instance.
             *
             * @param remote_peer : peer object representing the client that
             *                      disconnected. 
             *
             */
            virtual void on_disconnect(peer* remote_peer);

    };

}

#endif
