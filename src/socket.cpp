#include <assert.h>
#include <string>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <sys/epoll.h>
#include "tuxnet/log.h"
#include "tuxnet/socket.h"
#include "tuxnet/peer.h"

namespace tuxnet
{

    // Constructors. ----------------------------------------------------------

    // Constructor with local/remote saddrs.
    socket::socket(const layer4_protocol& proto, int epoll_max_events) : 
        m_local_saddr(nullptr), m_remote_saddr(nullptr), m_proto(proto), 
        m_fd(0), m_epoll_fd(0), m_epoll_events(nullptr), 
        m_epoll_maxevents(epoll_max_events),
        m_state(SOCKET_STATE_UNINITIALIZED)
    {
        m_fd = ::socket(AF_INET, SOCK_STREAM, layer4_to_proto(proto));
        m_epoll_events = new epoll_event[m_epoll_maxevents]();
    }

    // Destructor.
    socket::~socket()
    {
        if (m_fd == 0)
        {
            close(m_fd);
        }
        if (m_epoll_events != nullptr)
        {
            delete[] m_epoll_events;
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
        m_state = SOCKET_STATE_LISTENING;
        return true;
    }

    // Checks for any events on the socket.
    void socket::poll()
    {
        if (
            (m_state != SOCKET_STATE_STATELESS)
            and (m_state != SOCKET_STATE_LISTENING)
            and (m_state != SOCKET_STATE_CONNECTED)
        )
        {
            log::get()->error("Socket is not in a state in"
                " which it can be polled.");
            return;
        }
        int event_count = epoll_wait(
            m_epoll_fd, m_epoll_events, m_epoll_maxevents, -1);
        std::cout << "Got ";
        std::cout << event_count;
        std::cout << " events." << std::endl;
        for (int n_event = 0 ; n_event < event_count ; ++n_event)
        {
            if (
                (m_epoll_events[n_event].events & EPOLLERR)
                or (m_epoll_events[n_event].events & EPOLLHUP)
                or (not (m_epoll_events[n_event].events & EPOLLIN))
            ) 
            {
                /// @TODO handle these (fire event) and remove debug msg.
                std::cout << "epoll error." << std::endl;
                continue;
            }
            else if (m_epoll_events[n_event].data.fd == m_fd)
            {
                if (m_state == SOCKET_STATE_LISTENING)
                {
                    while(true)
                    {
                        if (m_local_saddr->get_protocol() == L3_PROTO_IP4)
                        {
                            sockaddr* in_addr = nullptr;
                            socklen_t in_len = 0;
                            int in_fd = accept(m_fd, in_addr, &in_len);
                            if (in_fd == -1)
                            {
                                if ((errno == EAGAIN) or (errno == EWOULDBLOCK))
                                {
                                    break;
                                }
                                else
                                {
                                    /// TODO: Could not accept, report error.
                                    break;
                                }
                            }
                            else
                            {
                                /// TODO monitor in_fd with epoll.
                                /// TODO call on_connect()
                                if (in_addr->sa_family == AF_INET)
                                {
                                    sockaddr_in* in_saddr = reinterpret_cast<sockaddr_in*>(
                                        in_addr
                                    );
                                    peer* my_peer = new peer(in_fd, *in_saddr);
                                    std::cout << "accepted" << std::endl;
                                }
                                else
                                {
                                    /// TODO: throw error about unsupported peer address family.
                                }
                                break;
                            }
                        }
                        else
                        {
                            /// TODO handle ipv6
                            break;
                        }
                    }
                }
            }
        }
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

