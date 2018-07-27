#include "tuxnet/config.h"

namespace tuxnet
{

    // Ctor(s) / dtor. --------------------------------------------------------
    
    /// Init config class static members.
    std::unique_ptr<config> config::m_instance;
    std::once_flag config::m_instance_allocated;

    // Constructor.
    config::config() : m_listen_socket_epoll_max_events(30),
        m_peer_socket_epoll_max_events(30)
    {
    }

    // Getters / setters. -----------------------------------------------------

    // Get pointer to config instance.
    config& config::get()
    {
        std::call_once(m_instance_allocated,[]{
            m_instance.reset(new config);
        });
        return *m_instance.get();
    }

    // Get epoll event buffer size for listen sockets.
    int const config::get_listen_socket_epoll_max_events()
    {
        return m_listen_socket_epoll_max_events;
    }

    // Get epoll event buffer size for peer sockets.
    int const config::get_peer_socket_epoll_max_events()
    {
        return m_peer_socket_epoll_max_events;
    }

}
