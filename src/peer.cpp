#include <netinet/in.h>
#include "tuxnet/peer.h"
#include "tuxnet/socket_address.h"

namespace tuxnet
{

    // IPV4 constructor.
    peer::peer(int fd, const sockaddr_in& in_addr) : m_fd(fd)
    {
        m_saddr = dynamic_cast<socket_address*>(
            new ip4_socket_address(in_addr)
        );
    }

    // IPV6 constructor.
    peer::peer(int fd, const sockaddr_in6& in_addr) : m_fd(fd)
    {
        m_saddr = dynamic_cast<socket_address*>(
            new ip6_socket_address(in_addr)
        );
    }

    // Destructor.
    peer::~peer()
    {
        m_fd = 0;
        if (m_saddr != nullptr)
        {
            delete m_saddr;
            m_saddr = nullptr;
        }
    }

}

