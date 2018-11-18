#include <assert.h>
#include <string>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include "tuxnet/log.h"
#include "tuxnet/socket.h"
#include "tuxnet/peer.h"
#include "tuxnet/server.h"
#include "tuxnet/event.h"
#include "tuxnet/config.h"

namespace tuxnet
{

    // Constructors. ----------------------------------------------------------

    // Constructor with local/remote saddrs.
    socket::socket(const layer4_protocol& proto) :
        m_epoll_client_events(nullptr),
        m_epoll_client_fd(0),
        m_epoll_listener_events(nullptr), 
        m_epoll_listener_fd(0), 
        m_listen_socket_fd(0),
        m_keepalive(true),
        m_keepalive_interval(5),
        m_keepalive_retry(3),
        m_keepalive_timeout(10),
        m_local_saddr(nullptr),
        m_peers({}),
        m_proto(proto),
        m_remote_saddr(nullptr), 
        m_server(nullptr),
        m_state(SOCKET_STATE_UNINITIALIZED)
    {
        m_listen_socket_fd = ::socket(AF_INET, SOCK_STREAM, layer4_to_proto(proto));
        m_epoll_listener_events = new epoll_event[
            config::get().get_listen_socket_epoll_max_events()]();
        /// @TODO create separate config option for client max events.
        epoll_event* event_buffer = new epoll_event[
            config::get().get_listen_socket_epoll_max_events()];
        m_epoll_client_events = new lockable<epoll_event*>(event_buffer);
    }

    // Destructor.
    socket::~socket()
    {
        close();
        if (m_epoll_listener_events != nullptr)
        {
            delete[] m_epoll_listener_events;
        }
        if (m_epoll_client_events != nullptr)
        {
            delete[] m_epoll_client_events;
        }
    }

    // Getters / setters. -----------------------------------------------------

    // Get client epoll event handler.
    const lockable<epoll_event*> & socket::get_client_epoll_event_handler() const
    {
        return std::cref(*m_epoll_client_events);
    }

    // Get client epoll event file descriptor.
    const int socket::get_client_epoll_fd() const
    {
        return m_epoll_client_fd;
    }

    // Gets whether or not keepalive is enabled for this socket.
    bool socket::get_keepalive() const
    {
        return m_keepalive;
    }

    // Returns the keepalive interval for this socket.
    int socket::get_keepalive_interval() const
    {
        return m_keepalive_interval;
    }

    // Returns the keepalive retries for this socket.
    int socket::get_keepalive_retry() const
    {
        return m_keepalive_retry;
    }

    // Returns the keepalive timeout for this socket.
    int socket::get_keepalive_timeout() const
    {
        return m_keepalive_timeout;
    }

    // Gets ip/port information for local side of the connection.
    const socket_address* const socket::get_local() const
    {
        return m_local_saddr;
    }

    // Gets the protocol used for this socket.
    layer4_protocol socket::get_proto() const
    {
        return m_proto;
    }

    // Gets ip/port information for remote side of the connection.
    const socket_address* const socket::get_remote() const
    {
        return m_remote_saddr;
    }

    // Sets whether or not keepalive should be enabled for this socket.
    void socket::set_keepalive(bool keepalive_enabled)
    {
        m_keepalive = keepalive_enabled;
    }

    // Sets the keepalive interval.
    void socket::set_keepalive_interval(int interval)
    {
        m_keepalive_interval = interval;
    }

    // Sets the keepalive retries.
    void socket::set_keepalive_retry(int retries)
    {
        m_keepalive_retry = retries;
    }

    // Sets the keepalive timeout.
    void socket::set_keepalive_timeout(int timeout)
    {
        m_keepalive_timeout = timeout;
    }

    // Public methods. --------------------------------------------------------

    // Binds the socket to an address/port pair.
    bool socket::bind(const socket_address* const saddr)
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
        log::get().error(std::string("Could not bind socket")
            + "(invalid/unset socket_address layer-3 protocol.)");
        return false;
    }

    // Starts listening on given address/port pair.
    bool socket::listen(const socket_address* const saddr, server* server_object)
    {
        // Bind the socket.
        if (socket::bind(saddr) != true) return false;
        // Listen on the socket.
        /**
         * @todo : configurable backlog with net.core.somaxconn as default.
         */
        if (::listen(m_listen_socket_fd,5) == -1)
        {
            std::string errstr = "Could not listen on socket (error ";
            errstr += std::to_string(errno) + " : ";
            errstr += strerror(errno);
            errstr += ").";
            log::get().error(errstr);
            return false;
        }
        // Make socket non-blocking.
        if (m_make_fd_nonblocking(m_listen_socket_fd) != true)
        {
            return false;
        }
        // Set up epoll notifications.
        m_epoll_listener_fd = create_event_listener();
        m_epoll_client_fd = create_event_listener();
        if (m_epoll_listener_fd == -1) return false;
        if (m_epoll_client_fd == -1) return false;
        if (event_monitor(m_listen_socket_fd, m_epoll_listener_fd) == true)
        {
            log::get().debug(
                "Created server event monitor (sock_fd="
                + std::to_string(m_listen_socket_fd) + ", epoll_fd="
                + std::to_string(m_epoll_listener_fd) + ")"
            );
            m_state = SOCKET_STATE_LISTENING;
            m_server = server_object;
            return true;
        }
        return false;
    }

    // Closes the socket.
    void socket::close()
    {
        m_state = SOCKET_STATE_CLOSING;
        if ((m_epoll_listener_fd != 0) and (m_listen_socket_fd != 0))
        {
            free_monitor(m_listen_socket_fd, m_epoll_listener_fd);
        }
        if (m_listen_socket_fd != 0) 
        {
            shutdown(m_listen_socket_fd, SHUT_RDWR);
            m_listen_socket_fd = 0;
        }
        for (auto it = m_peers.get().begin(); it != m_peers.get().end(); ++it)
        {
            peer* p = (*it).second;
            if (p != nullptr)
            {
                shutdown(p->get_fd(), SHUT_RDWR);
            }
            delete p;
            (*it).second = nullptr;
        }
        m_peers.atomic([](std::map<int,peer*>& pm){
            pm.clear();
        });
        m_state = SOCKET_STATE_CLOSED;
    }

    // Disconnects a remote peer.
    void socket::disconnect(peer* client)
    {
        client->disconnect();
    }

    // Checks for any incomming client events.
    bool socket::poll_clients()
    {
        int efd = m_epoll_client_fd;
        epoll_event* event_buffer = m_epoll_client_events->get();
        assert(efd != 0);
        assert(event_buffer != nullptr);
        /// @todo make last argument to epoll_wait configurable (timeout).
        m_epoll_client_events->lock();
        int event_count = epoll_wait(
            m_epoll_client_fd,
            event_buffer,
            config::get().get_listen_socket_epoll_max_events(), 
            -1);
        if (event_count == -1)
        {
            log::get().debug(std::string("Epoll error: ") + strerror(errno));
        }
        m_epoll_client_events->unlock();
        for (int n_event = 0 ; n_event < event_count ; ++n_event)
        {
            int event_fd = event_buffer[n_event].data.fd;
            // See if we can find a peer who owns this fd.
            auto conn_it = m_peers.get().find(event_fd);
            if (conn_it == m_peers.get().end())
            {
                // wtf?
                log::get().debug(
                    "Received event for a peer "
                    "I don't know anything about. (fd="
                    + std::to_string(event_fd) + ", n_event="
                    + std::to_string(n_event) + ")");
                continue;
            }
            peer* p = (*conn_it).second;
            if (p == nullptr)
            {
                // should not happen.
                log::get().debug(
                    "Received event for a peer with a nullptr handle.");
                continue;
            }
            if (
                (event_buffer[n_event].events & EPOLLERR)
                or (event_buffer[n_event].events & EPOLLHUP)
                or (not (event_buffer[n_event].events & EPOLLIN))
            )
            {
                p->disconnect();
            }
            on_receive(p);
        }
    }

    // Checks for any incomming server listening socket events.
    bool socket::poll_server()
    {
        if (
            (m_state != SOCKET_STATE_STATELESS)
            and (m_state != SOCKET_STATE_LISTENING)
            and (m_state != SOCKET_STATE_CONNECTED)
        )
        {
            log::get().error("Socket is not in a state in"
                " which it can be polled.");
            return false;
        }
        /* Call epoll_wait to fetch a bunch of events and
         * loop through them to handle them. */
        /// @todo make last argument to epoll_wait configurable (timeout).
        int event_count = epoll_wait(
            m_epoll_listener_fd, 
            m_epoll_listener_events, 
            config::get().get_listen_socket_epoll_max_events(), 
            -1);
        for (int n_event = 0 ; n_event < event_count ; ++n_event)
        {
            int event_fd = m_epoll_listener_events[n_event].data.fd;
            if (
                (m_epoll_listener_events[n_event].events & EPOLLERR)
                or (m_epoll_listener_events[n_event].events & EPOLLHUP)
                or (not (m_epoll_listener_events[n_event].events & EPOLLIN))
            ) 
            {
                close();
            }
            else if (event_fd == m_listen_socket_fd)
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
                        m_peers.lock();
                        m_peers.get().insert(std::make_pair(my_peer->get_fd(), my_peer));
                        m_peers.unlock();
                        if (m_server != nullptr)
                        {
                            on_connect(my_peer);
                        }
                    }
                }
            }
            else
            {
                /// @todo fixme
                log::get().error("Event received for unmonitored fd on server socket.");
            }
        }
       return true;
    }

    // Private methods. -------------------------------------------------------

    // Enables keepalive on the socket.
    bool socket::m_enable_keepalive(int fd)
    {
        if (m_keepalive != true) return true;
        int enable = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(int))
            == -1)
        {
            std::string errstr = "setsockopt(...SOL_SOCKET, SO_KEEPALIVE...)";
            errstr += " failed: ";
            errstr += strerror(errno);
            errstr += " (errno=" + std::to_string(errno)+", ";
            errstr += " fd=", std::to_string(fd);
            log::get().error(errstr);
            return false;
        }
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &m_keepalive_interval,
            sizeof(int)) == -1)
        {
            std::string errstr = "setsockopt(...IPPROTO_TCP, TCP_KEEPINTVL...";
            errstr += ") failed: ";
            errstr += strerror(errno);
            errstr += " (errno=" + std::to_string(errno)+", ";
            errstr += " fd=", std::to_string(fd);
            log::get().error(errstr);
            return false;  
        }
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &m_keepalive_retry,
            sizeof(int)) == -1)
        {
            std::string errstr = "setsockopt(...IPPROTO_TCP, TCP_KEEPCNT...";
            errstr += ") failed: ";
            errstr += strerror(errno);
            errstr += " (errno=" + std::to_string(errno)+", ";
            errstr += " fd=", std::to_string(fd);
            log::get().error(errstr);
            return false;   
        }
        if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &m_keepalive_timeout,
            sizeof(int)) == -1)
        {
            std::string errstr = "setsockopt(...IPPROTO_TCP, TCP_KEEPIDLE...)";
            errstr += " failed: ";
            errstr += strerror(errno);
            errstr += " (errno=" + std::to_string(errno)+", ";
            errstr += " fd=", std::to_string(fd);
            log::get().error(errstr);
            return false; 
        }
        return true;
    }

    // Bind socket to an IPv4 address.
    bool socket::m_ip4_bind()
    {
        if (m_local_saddr->get_protocol() == L3_PROTO_NONE)
        {
            log::get().error("No layer-3 protocol set for socket_address.");
            return false;
        }
        const ip4_socket_address* p4saddr = dynamic_cast<
            const ip4_socket_address*>(m_local_saddr);
        assert(p4saddr);
        const sockaddr_in saddr = p4saddr->get_sockaddr_in();
        int result = ::bind(m_listen_socket_fd, 
            reinterpret_cast<const sockaddr*>(&saddr), 
            sizeof(saddr));
        if (result == -1)
        {
            std::string errstr = "Could not bind socket (error ";
            errstr += std::to_string(errno);
            errstr += " : ";
            errstr += strerror(errno);
            errstr += ").";
            log::get().error(errstr);
            return false;
        }
        return m_make_fd_nonblocking(m_listen_socket_fd);
    }

    // Bind socket to an IPv6 address.
    bool socket::m_ip6_bind()
    {
        /// @todo : implement ipv6
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
            log::get().error(errmsg);
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
            log::get().error(errmsg);
            return false;
        }
        return true;
    }

    // Remove a peer, cleanup after a client disconnects.
    void socket::remove_peer(peer* client)
    {
        if (client == nullptr) return;
        // Fire event.
        on_disconnect(client);
        // Find peer.
        m_peers.lock();
        auto it = m_peers.get().begin();
        for (;it != m_peers.get().end();++it)
        {
            if ((*it).second == client) break;
        }
        // Delete peer if found.
        if (it != m_peers.get().end())
        {
            m_peers.get().erase(it);
        }
        m_peers.unlock();
        // Free peer.
        delete client;
        client = nullptr;
    }

    // Try to accept an incomming connection.
    peer* socket::m_try_accept()
    {
        if (m_local_saddr->get_protocol() == L3_PROTO_IP4)
        {
            sockaddr_in in_addr = {};
            socklen_t in_len = sizeof(sockaddr_in);
            int in_fd = accept(
                m_listen_socket_fd, 
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
                    log::get().error(errmsg);
                    return nullptr;
                }
            }
            else
            {
                if (m_enable_keepalive(in_fd) != true)
                {
                    log::get().error("Could not enable keepalive on peer"
                        "socket.");
                    if (in_fd > 0)
                    {
                        shutdown(in_fd, SHUT_RDWR);
                    }
                    return nullptr;
                }           
                peer* my_peer = new peer(in_fd, in_addr, this);
                if (my_peer->initialize() != true)
                {
                    delete my_peer;
                    my_peer = nullptr;
                }
                return my_peer;
            }
        }
        else
        {
            /// @todo handle ipv6
        }
        return nullptr;
    }

    // Events. ----------------------------------------------------------------

    void socket::on_receive(peer* client)
    {
        m_server->on_receive(client);
    }
    
    void socket::on_connect(peer* client)
    {
        m_server->on_connect(client);
    }

    void socket::on_disconnect(peer* client)
    {
        m_server->on_disconnect(client);
    }

}

