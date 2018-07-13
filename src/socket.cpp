#include <assert.h>
#include <string>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include "tuxnet/log.h"
#include "tuxnet/socket.h"
#include "tuxnet/peer.h"
#include "tuxnet/server.h"

namespace tuxnet
{

    // Constructors. ----------------------------------------------------------

    // Constructor with local/remote saddrs.
    socket::socket(const layer4_protocol& proto, int epoll_max_events) : 
        m_local_saddr(nullptr), m_remote_saddr(nullptr), m_proto(proto), 
        m_fd(0), m_epoll_fd(0), m_epoll_events(nullptr), 
        m_epoll_maxevents(epoll_max_events), m_server(nullptr),
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
        for (auto cur_peer = m_peers.begin(); 
            cur_peer != m_peers.end(); ++cur_peer)
        {
            if (cur_peer->second != nullptr)
            {
                delete (cur_peer->second);
                cur_peer->second = nullptr;
            }
        }
        peers().swap(m_peers);
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
    bool socket::listen(const socket_address* saddr, server* server_object)
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
        if (m_monitor_fd(m_fd) == true)
        {
            m_state = SOCKET_STATE_LISTENING;
            m_server = server_object;
            return true;
        }
        return false;
    }

    // Checks for any events on the socket.
    bool socket::poll()
    {
        bool result = false;
        if (
            (m_state != SOCKET_STATE_STATELESS)
            and (m_state != SOCKET_STATE_LISTENING)
            and (m_state != SOCKET_STATE_CONNECTED)
        )
        {
            log::get()->error("Socket is not in a state in"
                " which it can be polled.");
            return result;
        }
        /* Call epoll_wait to fetch a bunch of events and
         * loop through them to handle them. */
        int event_count = epoll_wait(
            m_epoll_fd, m_epoll_events, m_epoll_maxevents, -1);
        for (int n_event = 0 ; n_event < event_count ; ++n_event)
        {
            if (
                (m_epoll_events[n_event].events & EPOLLERR)
                or (m_epoll_events[n_event].events & EPOLLHUP)
                or (not (m_epoll_events[n_event].events & EPOLLIN))
            ) 
            {
                std::string errmsg = "epoll error: ";
                errmsg += strerror(errno);
                errmsg += " (";
                errmsg += std::to_string(errno);
                errmsg += ")";
                log::get()->error(errmsg);
                close(m_epoll_events[n_event].data.fd);
                continue;
            }
            else if (m_epoll_events[n_event].data.fd == m_fd)
            {
                /* An event came in on our socket.
                 * This is typically a connection attempt.
                 * If we're in a listening state, handle incomming
                 * connections and fire an on_connect event. */
                if (m_state == SOCKET_STATE_LISTENING)
                {
                    peer* my_peer = m_try_accept();
                    if (my_peer != nullptr)
                    {
                        m_peers.insert({my_peer->get_fd(), my_peer});
                        if (m_server != nullptr)
                        {
                            m_server->on_connect(my_peer);
                            result = true;
                        }
                    }
                }
            }
            else
            {
                /* An event came in on one of the peer sockets.
                 * This is typically peer data. */
                auto client_peer_it =  m_peers.find(
                    m_epoll_events[n_event].data.fd);
                if (client_peer_it == m_peers.end())
                {
                    std::string errstr = "Received an event with a file";
                    errstr += "descriptor number not found in the server's list";
                    errstr += " of peers. (fd = ";
                    errstr += std::to_string(m_epoll_events[n_event].data.fd);
                    errstr += ")";
                    return false;
                }
                if (m_state == SOCKET_STATE_LISTENING)
                {
                    if (m_server != nullptr)
                    {
                        m_server->on_receive(client_peer_it->second);
                    }
                }
                result = true;
            }
        }
        return result;
    }

    // Private methods. -------------------------------------------------------

    // Try to accept an incomming connection.
    peer* socket::m_try_accept()
    {
        if (m_local_saddr->get_protocol() == L3_PROTO_IP4)
        {
            sockaddr_in in_addr = {};
            socklen_t in_len = sizeof(sockaddr_in);
            int in_fd = accept(
                m_fd, 
                reinterpret_cast<sockaddr*>(&in_addr), 
                &in_len
            );
            if (in_fd == -1)
            {
                if ((errno == EAGAIN) or (errno == EWOULDBLOCK))
                {
                    return nullptr;
                }
                else
                {
                    std::string errmsg = "Could not accept a connection: ";
                    errmsg += strerror(errno);
                    errmsg += " (";
                    errmsg += std::to_string(errno);
                    errmsg += ")";
                    log::get()->error(errmsg);
                    return nullptr;
                }
            }
            else
            {
                if (m_monitor_fd(in_fd) == true)
                {
                    peer* my_peer = new peer(in_fd, in_addr);
                    return my_peer;
                }
            }
        }
        else
        {
            /// TODO handle ipv6
        }
        return nullptr;
    }

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
        return m_make_fd_nonblocking(m_fd);
    }

    // Bind socket to an IPv6 address.
    bool socket::m_ip6_bind()
    {
        /// @TODO : implement ipv6
    }

    // Attempts to make a file-descriptor non-blocking.
    bool socket::m_make_fd_nonblocking(int fd)
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags == -1)
        {
            std::string errmsg = "F_GETFL fcntl failed when attempting to ";
            errmsg += "make file-descriptor non-blocking: ";
            errmsg += strerror(errno);
            errmsg += " (errno=";
            errmsg += std::to_string(errno);
            errmsg += ", fd=";
            errmsg += std::to_string(fd);
            errmsg += ")";
            log::get()->error(errmsg);
            return false;
        }
        flags |= O_NONBLOCK;
        if (fcntl(fd, F_SETFL, flags) == -1)
        {
            std::string errmsg = "F_SETFL fcntl failed when attempting to ";
            errmsg += "make file-descriptor non-blocking: ";
            errmsg += strerror(errno);
            errmsg += " (errno=";
            errmsg += std::to_string(errno);
            errmsg += ", fd=";
            errmsg += std::to_string(fd);
            errmsg += ")";
            log::get()->error(errmsg);
            return false;
        }
        return true;
    }

    // Adds a file-descriptor to epoll monitoring.
    bool socket::m_monitor_fd(int fd)
    {
        epoll_event event = {};
        event.data.fd = fd;
        if (not m_make_fd_nonblocking(fd))
        {
            return false;
        }
        event.events = EPOLLIN | EPOLLET;
        if (epoll_ctl(
            m_epoll_fd, 
            EPOLL_CTL_ADD, 
            fd,
            &event) == -1)
        {
            std::string errmsg = "Could not add epoll event: ";
            errmsg += strerror(errno);
            errmsg += " (errno=";
            errmsg += std::to_string(errno);
            errmsg += ", epoll_fd=";
            errmsg += std::to_string(m_epoll_fd);
            errmsg += ", peer_fd=";
            errmsg += std::to_string(fd);
            errmsg += ")";
            log::get()->error(errmsg);
            return false;
        }
        return true;
    }
}

