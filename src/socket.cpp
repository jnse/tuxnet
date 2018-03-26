#include <string.h>
#include "tuxnet/log.h"
#include "tuxnet/socket.h"

namespace tuxnet
{

    // Constructors. ----------------------------------------------------------

    // Constructor with local/remote saddrs.
    socket::socket(const layer4_protocol& proto) : m_proto(proto), m_fd(0)
    {

    }

    // Getters. ---------------------------------------------------------------

    // Gets ip/port information for local side of the connection.
    const socket_address& socket::get_local() const
    {
        return m_local_saddr;
    }

    // Gets ip/port information for remote side of the connection.
    const socket_address& socket::get_remote() const
    {
        return m_remote_saddr;
    }

    // Gets the protocol used for this socket.
    const layer4_protocol socket::get_proto() const
    {
        return m_proto;
    }

    // Public methods. --------------------------------------------------------

    // Binds the socket to an address/port pair.
    bool socket::bind(const socket_address& saddr)
    {
        m_local_saddr = saddr;
        if (m_local_saddr.get_protocol() == L3_PROTO_IP4)
        {
            return ip4_bind(); 
        }
        else if (m_local_saddr.get_protocol() == L3_PROTO_IP6)
        {
            return ip6_bind();
        }
        log::get()->error(std::string("Could not bind socket")
            + "(invalid/unset socket_address layer-3 protocol.)");
        return false;
    }

    // Private methods. -------------------------------------------------------

    // Bind socket to an IPv4 address.
    bool socket::ip4_bind()
    {
        const sockaddr_in saddr = dynamic_cast<const ip4_socket_address*>(
            &m_local_saddr)->get_sockaddr_in();
        int result = ::bind(m_fd, reinterpret_cast<const sockaddr*>(&saddr), 
            sizeof(saddr));
        if (result == -1)
        {
            std::string errstr = "Could not bind socket (error ";
            errstr += std::to_string(errno);
            errstr += " : ";
            errstr += strerror(errno);
            errstr += ").";
            log::get()->error(errstr);
            return false;
        }
        return true;
    }

    // Bind socket to an IPv6 address.
    bool socket::ip6_bind()
    {
        /// @TODO : implement ipv6
    }

}

