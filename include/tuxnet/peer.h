#ifndef PEER_H_INCLUDE
#define PEER_H_INCLUDE

#include <netinet/in.h>
#include <sys/epoll.h>
#include <unordered_map>
#include <atomic>
#include <thread>
#include "tuxnet/socket_address.h"

namespace tuxnet
{

    // Forward declaration of socket.
    class socket;

    /// Enum for the different states a peer can be in.
    enum peer_state
    {
        PEER_STATE_UNINITIALIZED=0,
        PEER_STATE_CONNECTED,
        PEER_STATE_CLOSING,
        PEER_STATE_CLOSED
    };

    /// Peer forward declaration.
    class peer;

    /// Collection of peers.
    typedef std::vector<peer*> peers;

    /**
     * Peer.
     *
     * This object represents a remote peer.
     */
    class peer
    {

        /// epoll event buffer.
        epoll_event* m_epoll_events;
        /// Peer state.
        std::atomic<peer_state> m_state;
        /// Socket file descriptor.
        int m_fd;
        /// Event file descriptor.
        int m_epoll_fd;
        /// IP and port of peer.
        socket_address* m_saddr;
        /// Pointer to parent socket.
        socket* const m_socket;
        /// Thread to poll socket.
        std::thread* m_poll_thread;

        public:

            // ctor(s) / dtor. ------------------------------------------------

            /**
             * IPV4 constructor.
             *
             * @param fd : File descriptor associated with connection.
             * @param in_addr : sockaddr for peer connection.
             * @param parent : pointer to socket owning this connection.
             */
            peer(int fd, const sockaddr_in& in_addr, socket* const parent);

            /**
             * IPV6 constructor.
             *
             * @param fd : File descriptor associated with connection.
             * @param in_addr : sockaddr for peer connection.
             * @param parent : pointer to socket owning this connection.
             */
            peer(int fd, const sockaddr_in6& in_addr, socket* const parent);

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

            /**
             * Get the peer state.
             * @ return Returns the state the peer connection is in.
             */
            peer_state const get_state() const;

            // Methods. -------------------------------------------------------

            /**
             * Sets up peer for event monitoring.
             * @return Returns true on success, false on failure.
             */
            bool initialize();

            /**
             * Polls the remote peer to see if it sent any date.
             */
            void poll();

            /**
             * Reads up to a given number of characters from the 
             * peer socket.
             *
             * @param characters : Number of characters to read.
             * @return Returns text received.
             */
            std::string read_string(int characters);

            /**
             * Reads string until given token is received.
             *
             * @param token : substring to look for in received data.
             * @return Returns data received up-to and including token.
             */
            std::string read_string_until(std::string token);

            /**
             * Reads a line of text.
             *
             * @return Returns received line of text.
             */
            std::string read_line();

            /**
             * Reads everything the client sent.
             */
            std::string read_all();

            /// @todo Add functions for reading/writing raw bytes.

            /**
             * Send data to the remote peer.
             *
             * @param text : Text to send.
             */
            void write_string(std::string text);

            /// Close connection to this peer.
            void disconnect();

    };

}

#endif
