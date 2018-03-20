#include "tuxnet/socket.h"

namespace tuxnet
{

    // Constructors. ----------------------------------------------------------

    // Default/empty constructor.
    ip4_socket::ip4_socket() : m_proto(L4_PROTO_NONE)
    {
    }

    // Constructor with local/remote saddrs.
    ip4_socket::ip4_socket(const ip4_socket_address& local, 
        const ip4_socket_address& remote, layer4_protocol proto) : 
        m_local_saddr(local), m_remote_saddr(remote), m_proto(proto)
    {
    }

    // Setters. ---------------------------------------------------------------

    // Sets ip/port information for local side of the connection.
    void ip4_socket::set_local(const ip4_socket_address& saddr)
    {
        m_local_saddr = saddr;
    }

    // Sets ip/port information for remote side of the connection.
    void ip4_socket::set_remote(const ip4_socket_address& saddr)
    {
        m_remote_saddr = saddr;
    }

    // Sets the protocol to be used for this socket.
    void ip4_socket::set_proto(layer4_protocol proto)
    {
        m_proto = proto;
    }

    // Getters. ---------------------------------------------------------------

    // Gets ip/port information for local side of the connection.
    const ip4_socket_address& ip4_socket::get_local() const
    {
        return m_local_saddr;
    }

    // Gets ip/port information for remote side of the connection.
    const ip4_socket_address& ip4_socket::get_remote() const
    {
        return m_remote_saddr;
    }

    // Gets the protocol used for this socket.
    const layer4_protocol ip4_socket::get_proto() const
    {
        return m_proto;
    }

}

