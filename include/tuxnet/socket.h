/**
 * Network socket definition.
 **/

#ifndef SOCKET_H_INCLUDE
#define SOCKET_H_INCLUDE

#include "tuxnet/socket_address.h"

namespace tuxnet
{

    /**
     * Network socket.
     */
    class ip4_socket
    {

        // Private member variables. ------------------------------------------

        /// Stores the local address/port pair.
        ip4_socket_address m_local_saddr;

        /// Stores the remote address/port pair.
        ip4_socket_address m_remote_saddr;


        public:

            // Constructors. --------------------------------------------------

            // Default/empty constructor.
            ip4_socket();

            /** 
             * Constructor with local/remote saddrs.
             * @param local : Socket address object with ip/port information 
             *                for the local end of the connection.
             * @param remote : Socket address object with ip/port information 
             *                 for the remote end of the connection.
             */
            ip4_socket(const ip4_socket_address& local, 
                const ip4_socket_address& remote);

            // Setters. -------------------------------------------------------

            /**
             * Sets ip/port information for local side of the connection.
             * @param saddr : socket_address object containing ip/port 
             *                information for local end of the connection.
             */
            void set_local(const ip4_socket_address& saddr);

            /**
             * Sets ip/port information for remote side of the connection.
             * @param saddr : socket_address object containing ip/port 
             *                information for remote end of the connection.
             */
            void set_remote(const ip4_socket_address& saddr);

            // Getters. -------------------------------------------------------

            /**
             * Gets ip/port information for local side of the connection.
             * @return Returns socket_address object containing ip/port 
             *         information for local end of the connection.
             */
            const ip4_socket_address& get_local() const;

            /**
             * Gets ip/port information for remote side of the connection.
             * @return Returns socket_address object containing ip/port 
             *         information for remote end of the connection.
             */
            const ip4_socket_address& get_remote() const;

    };

}

#endif

