#include "tuxnet/config.h"

namespace tuxnet
{

    // Ctor(s) / dtor. --------------------------------------------------------
    
    /// Init config class static members.
    std::unique_ptr<config> config::m_instance;
    std::once_flag config::m_instance_allocated;

    // Constructor.
    config::config() : m_client_max_threads(10), m_client_min_threads(10),
        m_listen_socket_epoll_max_events(30), 
        m_peer_socket_epoll_max_events(30), m_server_max_threads(10), 
        m_server_min_threads(10)
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

    // Get minimum number of client threads.
    int const config::get_client_max_threads()
    {
        return m_client_max_threads;
    }

    // Get maximum number of client threads.
    int const config::get_client_min_threads()
    {
        return m_client_min_threads;
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

    // Get max server threads.
    int const config::get_server_max_threads()
    {
        return m_server_max_threads;
    }

    // Get min server threads
    int const config::get_server_min_threads()
    {
        return m_server_min_threads;
    }


}
