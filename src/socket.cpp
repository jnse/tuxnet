#include <assert.h>
#include <string>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "tuxnet/log.h"
#include "tuxnet/socket.h"

namespace tuxnet
{

    // Constructors. ----------------------------------------------------------

    // Constructor with local/remote saddrs.
    socket::socket(const layer4_protocol& proto) : 
        m_local_saddr(nullptr), m_remote_saddr(nullptr), m_proto(proto), 
        m_fd(0), m_epoll_fd(0), m_epoll_events(nullptr)
    {
        m_fd = ::socket(AF_INET, SOCK_STREAM, layer4_to_proto(proto));
    }

    // Destructor.
    socket::~socket()
    {
        if (m_fd == 0)
        {
            close(m_fd);
        }
    }

    // Getters. ---------------------------------------------------------------

    // Gets ip/port information for local side of the connection.
    const socket_address* socket::get_local() const
    {
        return m_local_saddr;
    }

    // Gets ip/port information for remote side of the connection.
    const socket_address* socket::get_remote() const
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
    bool socket::bind(const socket_address* saddr)
    {
        m_local_saddr = saddr;
        if (m_local_saddr->get_protocol() == L3_PROTO_IP4)
        {
            return m_ip4_bind(); 
        }
        else if (m_local_saddr->get_protocol() == L3_PROTO_IP6)
        {
            return m_ip6_bind();
        }
        log::get()->error(std::string("Could not bind socket")
            + "(invalid/unset socket_address layer-3 protocol.)");
        return false;
    }

    // Starts listening on given address/port pair.
    bool socket::listen(const socket_address* saddr)
    {
        // Bind the socket.
        if (socket::bind(saddr) != true) return false;
        // Listen on the socket.
        /**
         * @TODO : configurable backlog with net.core.somaxconn as default.
         */
        if (::listen(m_fd,5) == -1)
        {
            std::string errstr = "Could not listen on socket (error ";
            errstr += std::to_string(errno) + " : ";
            errstr += strerror(errno);
            errstr += ").";
            log::get()->error(errstr);
            return false;
        }
        // Set up epoll notifications.
        m_epoll_fd = epoll_create1(EPOLL_CLOEXEC);
        if (m_epoll_fd == -1)
        {
            std::string errstr = "epoll_create1 failed. (error ";
            errstr += std::to_string(errno) + " : ";
            errstr += strerror(errno);
            errstr += ").";
            log::get()->error(errstr);
            return false; 
        }
        struct epoll_event event = {};
        event.data.fd = m_fd;
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_fd, &event) != 0)
        {
            std::string errstr = "EPOLL_CTL_ADD failed. (error ";
            errstr += std::to_string(errno) + " : ";
            errstr += strerror(errno);
            errstr += ").";
            log::get()->error(errstr);
            return false;
        }
        /// TODO: finish epoll setup - split up in separate function, make maxevents configurable.
        //m_events = 
        return true;
    }

    // Checks for any events on the socket.
    void socket::poll()
    {
        
    }

    // Private methods. -------------------------------------------------------

    // Bind socket to an IPv4 address.
    bool socket::m_ip4_bind()
    {
        if (m_local_saddr->get_protocol() == L3_PROTO_NONE)
        {
            log::get()->error("No layer-3 protocol set for socket_address.");
            return false;
        }
        const ip4_socket_address* p4saddr = dynamic_cast<
            const ip4_socket_address*>(m_local_saddr);
        assert(p4saddr);
        const sockaddr_in saddr = p4saddr->get_sockaddr_in();
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
    bool socket::m_ip6_bind()
    {
        /// @TODO : implement ipv6
    }

}

