#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include "tuxnet/log.h"
#include "tuxnet/event.h"
#include "tuxnet/socket_address.h"
#include "tuxnet/socket.h"
#include "tuxnet/peer.h"
#include "tuxnet/config.h"

namespace tuxnet
{

    // IPV4 constructor.
    peer::peer(int fd, const sockaddr_in& in_addr, socket* const parent) : 
        m_fd(fd), m_epoll_fd(0), m_poll_thread(nullptr),
        m_socket(parent), 
        m_state(PEER_STATE_UNINITIALIZED)
    {
        m_saddr = dynamic_cast<socket_address*>(
            new ip4_socket_address(in_addr)
        );
        m_epoll_events = new epoll_event[
            config::get().get_listen_socket_epoll_max_events()]();
    }

    // IPV6 constructor.
    /// @todo fixme
    peer::peer(int fd, const sockaddr_in6& in_addr, socket* const parent) : 
        m_fd(fd), m_socket(parent), m_state(PEER_STATE_UNINITIALIZED)
    {
        m_saddr = dynamic_cast<socket_address*>(
            new ip6_socket_address(in_addr)
        );
        m_state = PEER_STATE_CONNECTED;
    }

    // Destructor.
    peer::~peer()
    {
        if (m_poll_thread != nullptr)
        {
            delete m_poll_thread;
            m_poll_thread = nullptr;
        }
        if ((m_fd != 0) and (m_epoll_fd != 0))
        {
            free_monitor(m_fd, m_epoll_fd);
            m_epoll_fd = 0;
        } 
        if (m_fd != 0)
        {
            shutdown(m_fd, SHUT_RDWR);
            ::close(m_fd);
            m_fd = 0;
        }
        if (m_saddr != nullptr)
        {
            delete m_saddr;
            m_saddr = nullptr;
        }
    }

    // Getters. ---------------------------------------------------------------

    // Get file descriptor.
    int peer::get_fd()
    {
        return m_fd;
    }

    // Get socket address.
    socket_address* peer::get_saddr()
    {
        return m_saddr;
    }

    // Get peer state.
    peer_state const peer::get_state() const
    {
        return m_state;
    }

    // Methods. ---------------------------------------------------------------

    // Sets up peer for event monitoring.
    bool peer::initialize()
    {
        m_epoll_fd = create_event_listener();
        if (m_epoll_fd == -1) return false;
        if (event_monitor(m_fd, m_epoll_fd) != true) return false;
        m_state = PEER_STATE_CONNECTED;
        m_poll_thread = new std::thread([=](){
            while(m_state == PEER_STATE_CONNECTED)
            {
                poll();
            }
        });
        m_poll_thread->detach();
        return true;
    }

    void peer::poll()
    {
        if (m_state != PEER_STATE_CONNECTED) return;
        /// @todo make last argument to epoll_wait configurable (timeout).
        int event_count = epoll_wait(
            m_epoll_fd, 
            m_epoll_events, 
            config::get().get_listen_socket_epoll_max_events(), 
            -1);
        if (event_count == -1)
        {
            disconnect();
            return;
        }
        for (int n_event = 0 ; n_event < event_count ; ++n_event)
        {
            int event_fd = m_epoll_events[n_event].data.fd;
            if (
                (m_epoll_events[n_event].events & EPOLLERR)
                or (m_epoll_events[n_event].events & EPOLLHUP)
                or (not (m_epoll_events[n_event].events & EPOLLIN))
            )
            {
                disconnect();
                return;
            }
            if (event_fd == m_fd)
            {
                m_socket->on_receive(this);
            }
            else
            {
                log::get().error("fd mismatch on peer socket.");
            }
        }
    }

    // Reads up to a given number of characters into string.
    std::string peer::read_string(int characters)
    {
        if (m_state != PEER_STATE_CONNECTED) return "";
        char buffer[characters];
        std::string result;
        if (m_fd == 0)
        {
            log::get().error("Read operation on a closed socket.");
            return "";
        }
        int read_so_far = 0;
        while (read_so_far < characters)
        {
            if (m_state != PEER_STATE_CONNECTED) return result;
            int count = recv(
                m_fd, 
                &buffer, 
                sizeof(buffer) * sizeof(char),
                MSG_DONTWAIT);
            if (count > 0)
            {
                read_so_far += count;
                buffer[read_so_far] = 0;
                result += buffer;
            }
            else
            {
                if (errno == EAGAIN)
                {
                    continue;
                }
                else
                {
                    // Client disconnected.
                    disconnect();
                    return result;
                }
            }
        }
        return result;
    }

    // Read string until token.
    std::string peer::read_string_until(std::string token)
    {
        if (m_state != PEER_STATE_CONNECTED) return "";
        char buffer;
        std::string result;
        if (m_fd == 0)
        {
            log::get().error("Read operation on a closed socket.");
            return "";
        }
        while (true)
        {
            if (m_state != PEER_STATE_CONNECTED) return result;
            int count = read(m_fd, &buffer, 1 * sizeof(char));
            if (count > 0)
            {
                if (result.find(token) != std::string::npos)
                {
                    break;
                }
                else
                {
                    result += buffer;
                }
            }
            else
            {
                if (errno == EAGAIN)
                {
                    continue;
                }
                else 
                {
                    // Client disconnected.
                    disconnect();
                    return result;
                }
            }
        }
        return result;
    }

    // Reads a line of text.
    std::string peer::read_line()
    {
        if (m_state != PEER_STATE_CONNECTED) return "";
        char buffer;
        std::string result;
        if (m_fd == 0)
        {
            log::get().error("Read operation on a closed socket.");
            return "";
        }
        while (true)
        {
            if (m_state != PEER_STATE_CONNECTED) return result;
            int count = read(m_fd, &buffer, 1 * sizeof(char));
            if (count > 0)
            {
                if ((buffer == '\n') or (buffer == '\r'))
                {
                    if (result.length() > 0)
                    {
                        break;
                    }
                }
                else
                {
                    result += buffer;
                }
            }
            else
            {
                if (errno == EAGAIN)
                {
                    continue;
                }
                else
                {
                    // Client disconnected.
                    disconnect();
                    return result;
                }
            }
        }
        return result;
    }

    // Reads everything the client sent.
    std::string peer::read_all()
    {
        if (m_state != PEER_STATE_CONNECTED) return "";
        char buffer;
        std::string result;
        if (m_fd == 0)
        {
            log::get().error("Read operation on a closed socket.");
            return "";
        }
        while (true)
        {
            if (m_state != PEER_STATE_CONNECTED) return result;
            int count = recv(
                m_fd, 
                &buffer, 
                sizeof(buffer) * sizeof(char),
                MSG_DONTWAIT);
            if (count > 0)
            {
                if (buffer == 0) 
                {
                    break;
                }
                else
                {
                    result += buffer;
                }
            }
            else
            {
                if (errno == EAGAIN)
                {
                    break;
                }
                else
                {
                    // Client disconnected.
                    disconnect();
                    return result;
                }
            }
        }
        return result;
    }


    void peer::write_string(std::string text)
    {
        if (m_state != PEER_STATE_CONNECTED) return;
        if (m_fd == 0) return;
        if (::send(m_fd, text.c_str(), text.length(), MSG_NOSIGNAL) 
                == -1)
        {
            if (errno == EPIPE)
            {
                // Broken pipe. Lost connection mid-write.
                disconnect();
                return;
            }
            std::string errstr = "Could not write to peer: ";
            errstr += strerror(errno);
            errstr += " (errno=" + std::to_string(errno) + ")";
            log::get().error(errstr);
            disconnect();
        }
    }

    // Close connection to this peer.
    void peer::disconnect()
    {
        if (m_state == PEER_STATE_CLOSING) return;
        m_state = PEER_STATE_CLOSING;
        if (m_poll_thread != nullptr)
        {
            if (m_poll_thread->joinable() == true)
            {
                m_poll_thread->join();
            }
        }
        m_socket->remove_peer(this);
    }

}

