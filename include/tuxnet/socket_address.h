#ifndef SOCKET_ADDRESS_H_INCLUDE
#define SOCKET_ADDRESS_H_INCLUDE

#include <vector>
#include <netinet/in.h>
#include "tuxnet/protocol.h"
#include "tuxnet/ip_address.h"

namespace tuxnet
{

    /**
     * Base class for IPv4 and IPv6 socket address objects.
     */
    class socket_address
    {

        // Protected member variables. ----------------------------------------
        
        protected:

            /// Stores the layer3 protocol of this socket address.
            layer3_protocol m_proto;

        public:

            // Constructors / destructor. -------------------------------------

            /// Default/empty constructor.
            socket_address();

            /**
             * Copy constructor.
             * @param other : socket_address to be copied into new object.
             */
            socket_address(const socket_address& other);

            /** 
             * Constructor with initializing protocol.
             * @param proto : Layer3 protocol to set for this socket address.
             */
            socket_address(const layer3_protocol& proto);

            /// Destructor.
            virtual ~socket_address();

            // Setters. -------------------------------------------------------

            /**
             * Set layer3 protocol.
             * @param proto Layer 3 protocol to use for this socket address.
             */
            void set_protocol(const layer3_protocol proto);

            // Getters. -------------------------------------------------------

            /**
             * Get layer3 protocol.
             * @returns Returns the layer3 protocol used by this 
             *          socket address object.
             */
            const layer3_protocol get_protocol() const;

    };

    /*************************************************************************/

    /// Collection of socket_address objects.
    typedef std::vector<socket_address*> socket_addresses;

    /*************************************************************************/
    
    /**
     * Class to hold an IPv4 address/port pair.
     */
    class ip4_socket_address : public socket_address
    {

        // Private member variables. ------------------------------------------

        // Internal storage of ipv4 address.
        ip4_address m_ip;

        // Internal storage of port number.
        in_port_t m_port;

        public:

            // Constructors. --------------------------------------------------

            /// Default/empty constructor.
            ip4_socket_address();

            /// Copy constructor.
            ip4_socket_address(const ip4_socket_address& other);

            /// Constructor with IPv4 IP and port.
            ip4_socket_address(const ip4_address& ip, int port);

            /// Constructor with sockaddr_in struct.
            ip4_socket_address(const sockaddr_in& saddr);

            // Setters. -------------------------------------------------------

            /**
             * Sets the IPV4 address and port of this socket_address object using
             * a populated sockadrr_in struct.
             */
            void set(const sockaddr_in& saddr);

            /**
             * Sets the IPv4 address of this socket_address object.
             *
             * @param ip : IPv4 address to set.
             */
            void set_ip(const ip4_address& ip);

            /** 
             * Sets the port number of this socket_address object.
             *
             * @param port : Port number to set.
             */
            void set_port(in_port_t port);

            // Getters. -------------------------------------------------------
        
            /**
             * Gets the IPv4 address associated with this socket_address object.
             *
             * @return Returns the IPv4 address associated with this object.
             */
            const ip4_address& get_ip() const;

            /**
             * Gets the port number associated with this socket_address object.
             *
             * @return Returns the port number associated with this object.
             */
            const in_port_t get_port() const;

            /**
             * Gets a sockaddr_in struct populated with the IP address and port.
             */
            const sockaddr_in get_sockaddr_in() const;

    };

    /*************************************************************************/
    
    /**
     * IPV6 socket_address object (unimplemented).
     */
    class ip6_socket_address : public socket_address
    {
        /// @TODO Add IPv6 support.
    };

}

#endif

