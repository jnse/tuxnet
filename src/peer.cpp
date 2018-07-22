#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include "tuxnet/log.h"
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
        if (m_fd != 0)
        {
            close(m_fd);
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

    // Methods. ---------------------------------------------------------------

    // Reads up to a given number of characters into string.
    std::string peer::read_string(int characters)
    {
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
                    std::string errstr = "Socket read error : ";
                    errstr += strerror(errno);
                    errstr += " (errno="  + std::to_string(errno);
                    errstr += ")";
                    log::get().error(errstr);
                    break;
                }
            }
        }
        return result;
    }

    // Read string until token.
    std::string peer::read_string_until(std::string token)
    {

    }

    // Reads a line of text.
    std::string peer::read_line()
    {
        char buffer;
        std::string result;
        if (m_fd == 0)
        {
            log::get().error("Read operation on a closed socket.");
            return "";
        }
        while (true)
        {
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
                else if (errno == 0)
                {
                    // Client disconnected.
                    /// @todo: call disconnect.
                    break;
                }
                else
                {
                 
                    std::string errstr = "Socket read error : ";
                    errstr += strerror(errno);
                    errstr += " (errno="  + std::to_string(errno);
                    errstr += ")";
                    log::get().error(errstr);
                    break;
                }
            }
        }
        return result;
    }

}

