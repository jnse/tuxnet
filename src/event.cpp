#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <string>
#include "tuxnet/log.h"
#include "tuxnet/event.h"

namespace tuxnet
{

    // Creates an event listener. 
    int create_event_listener()
    {
        int epoll_fd = -1;
        while (true)
        {
            epoll_fd = epoll_create1(EPOLL_CLOEXEC);
            if (epoll_fd == -1)
            {
                if (errno == EMFILE)
                {
                    std::string errstr = "Can't epoll_create1 : ";
                    errstr += "too many open files.";
                    errstr += "trying again in a second...";
                    sleep(1);
                    continue;
                }
                else
                {
                    std::string errstr = "epoll_create1 failed. (error ";
                    errstr += std::to_string(errno) + " : ";
                    errstr += strerror(errno);
                    errstr += ").";
                    log::get().error(errstr);
                }
            }
            break;
        }
        return epoll_fd;
    }

    // Sets up an event listener to monitor a socket fd.
    bool event_monitor(int socket_fd, int epoll_fd)
    {
        assert(socket_fd);
        assert(epoll_fd);
        epoll_event event = {};
        event.data.fd = socket_fd;
        event.events = EPOLLIN | EPOLLEXCLUSIVE;
        if (epoll_ctl(
            epoll_fd, 
            EPOLL_CTL_ADD, 
            socket_fd,
            &event) == -1)
        {
            std::string errmsg = "Could not add epoll event: ";
            errmsg += strerror(errno);
            errmsg += " (errno=";
            errmsg += std::to_string(errno);
            errmsg += ", epoll_fd=";
            errmsg += std::to_string(epoll_fd);
            errmsg += ", peer_fd=";
            errmsg += std::to_string(socket_fd);
            errmsg += ")";
            log::get().error(errmsg);
            return false;
        }
        return true;
    }

    // Frees an event listener.
    void free_monitor(int socket_fd, int epoll_fd)
    {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket_fd, nullptr);
        ::close(epoll_fd);
    }

}
