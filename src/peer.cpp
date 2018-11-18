#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
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
        m_fd(fd), m_socket(parent), m_state(PEER_STATE_UNINITIALIZED)
    {
        m_saddr = dynamic_cast<socket_address*>(
            new ip4_socket_address(in_addr)
        );
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
        if (m_socket == nullptr) return false;
        int epoll_fd = m_socket->get_client_epoll_fd();
        if (epoll_fd <= 0) return false;
        if (event_monitor(m_fd, epoll_fd) != true) return false;
        log::get().debug(
            "Created peer event monitor (sock_fd=" + std::to_string(m_fd)
            + ", epoll_fd=" + std::to_string(epoll_fd) + ")");
        m_state = PEER_STATE_CONNECTED;
        std::thread thread([this](){
            while (this->m_state == PEER_STATE_CONNECTED)
            {
                this->poll();
            }
        });
        thread.detach();
        return true;
    }

    void peer::poll()
    {
        /*
        int epoll_fd = m_socket->get_client_epoll_fd();
        lockable<epoll_event** event_buffer = m_socket->get_client_epoll_event_handler();
        if (epoll_fd == 0) return;
        if (event_buffer == nullptr) return;
        if (m_state != PEER_STATE_CONNECTED) return;
        */
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
        m_socket->remove_peer(this);
    }

}

