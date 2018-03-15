#ifndef SOCKET_ADDRESS_H_INCLUDE
#define SOCKET_ADDRESS_H_INCLUDE

#include <netinet/in.h>
#include "tuxnet/ip_address.h"

namespace tuxnet
{

    /**
     * Class to hold an IPv4 address/port pair.
     */
    class ip4_socket_address
    {

        // Private member functions. ------------------------------------------

        // Internal storage of ipv4 address.
        ip4_address m_ip;

        // Internal storage of port number.
        in_port_t m_port;

        // Constructors. ------------------------------------------------------

        /// Default/empty constructor.
        ip4_socket_address();

        /// Copy constructor.
        ip4_socket_address(const ip4_socket_address& other);

        /// Constructor with IPv4 IP and port.
        ip4_socket_address(const ip4_address& ip, in_port_t port);

        /// Constructor with sockaddr_in struct.
        ip4_socket_address(const sockaddr_in& saddr);

        // Setters. -----------------------------------------------------------

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

        // Getters. -----------------------------------------------------------
        
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
}

#endif

