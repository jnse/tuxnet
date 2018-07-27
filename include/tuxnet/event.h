#ifndef TUXNET_EVENT_H_INCLUDE
#define TUXNET_EVENT_H_INCLUDE

namespace tuxnet
{

    /**
     * Creates an event listener.
     *
     * @return Returns the listener file descriptor on success.
     * @return Returns -1 on failure.
     */
    int create_event_listener();

    /**
     * Monitor a socket file-descriptor for events.
     *
     * @param socket_fd : Socket file-descriptor.
     * @param epoll_fd : Pointer to epoll file descriptor.
     * @return Returns true on success.
     */
    bool event_monitor(int socket_fd, int epoll_fd);

}

#endif
