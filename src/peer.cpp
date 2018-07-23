#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include "tuxnet/log.h"
#include "tuxnet/peer.h"
#include "tuxnet/socket_address.h"
#include "tuxnet/socket.h"

namespace tuxnet
{

    // IPV4 constructor.
    peer::peer(int fd, const sockaddr_in& in_addr, socket* const parent) : 
        m_fd(fd), m_socket(parent), m_state(PEER_STATE_UNINITIALIZED)
    {
        m_saddr = dynamic_cast<socket_address*>(
            new ip4_socket_address(in_addr)
        );
        m_state = PEER_STATE_CONNECTED;
    }

    // IPV6 constructor.
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
            int count = read(m_fd, &buffer, sizeof(buffer) * sizeof(char));
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

    void peer::write_string(std::string text)
    {
        if (m_state != PEER_STATE_CONNECTED) return;
        if (::write(m_fd, text.c_str(), text.length()) == -1)
        {
            std::string errstr = "Could not write to peer: ";
            errstr += strerror(errno);
            errstr += " (errno=" + std::to_string(errno) + ")";
            log::get().error(errstr);
        }
    }

    // Close connection to this peer.
    void peer::disconnect()
    {
        m_state = PEER_STATE_CLOSING;
        m_socket->disconnect(this);
    }

}

