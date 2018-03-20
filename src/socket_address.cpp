#include "tuxnet/socket_address.h"

namespace tuxnet
{

    // Constructors. ----------------------------------------------------------

    // Default/empty constructor.
    ip4_socket_address::ip4_socket_address() : m_port(0)
    {

    }

    // Copy constructor.
    ip4_socket_address::ip4_socket_address(const ip4_socket_address& other)
    {
        m_ip = other.m_ip;
        m_port = other.m_port;
    }

    // Constructor with IPv4 IP and port.
    ip4_socket_address::ip4_socket_address(const ip4_address& ip, in_port_t port)
    {
        m_ip = ip;
        m_port = port;
    }

    // Constructor with sockaddr_in struct.
    ip4_socket_address::ip4_socket_address(const sockaddr_in& saddr)
    {
        ip4_socket_address();
        m_ip.set(saddr.sin_addr);
        m_port = saddr.sin_port;
    }

    // Setters. ---------------------------------------------------------------

    // Set IP and port from sockaddr_in object.
    void ip4_socket_address::set(const sockaddr_in& saddr)
    {
        m_ip.set(saddr.sin_addr);
        m_port = saddr.sin_port;
    }

    // Sets IP.
    void ip4_socket_address::set_ip(const ip4_address& ip)
    {
        m_ip = ip;
    }

    // Sets port.
    void ip4_socket_address::set_port(in_port_t port)
    {
        m_port = port;
    }

    // Getters. ---------------------------------------------------------------

    // Get IP.
    const ip4_address& ip4_socket_address::get_ip() const
    {
        return m_ip;
    }

    // Get port.
    const in_port_t ip4_socket_address::get_port() const
    {
        return m_port;
    }

    // Get populated sockaddr_in struct.
    const sockaddr_in ip4_socket_address::get_sockaddr_in() const
    {
        sockaddr_in result = {};
        result.sin_addr = m_ip.as_in_addr();
        result.sin_port = m_port;
        return result;
    }

}
