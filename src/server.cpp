#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <stdexcept>
#include <system_error>
#include <future>
#include "tuxnet/config.h"
#include "tuxnet/string.h"
#include "tuxnet/server.h"
#include "tuxnet/socket.h"
#include "tuxnet/log.h"

namespace tuxnet
{

    // Ctor(s) / dtor. --------------------------------------------------------

    // Constructor.
    server::server() : 
        m_keepalive(true), 
        m_keepalive_interval(5),
        m_keepalive_retries(3), 
        m_keepalive_timeout(10),
        m_listen_sockets({})
    {
        
    }

    // Destructor
    server::~server()
    {
        m_listen_sockets.lock();
        for (auto it = m_listen_sockets.get().begin(); 
            it != m_listen_sockets.get().end(); ++it)
        {
            delete (*it);
        }
        sockets().swap(m_listen_sockets.get());
        m_listen_sockets.unlock();
    }

    // Private member functions. ----------------------------------------------

    // Call poll on a single socket asynchronously.
    bool server::m_poll_single(socket* const sock)
    {
        if (sock == nullptr) return false;
        while(sock->poll() == true){ }
        return true;
    }

    // Methods. ---------------------------------------------------------------

    // Configures TCP keepalive settings.
    void server::configure_keepalive(bool enabled, int timeout, int interval,
        int retries)
    {
        m_keepalive = enabled;
        m_keepalive_timeout = timeout;
        m_keepalive_interval = interval;
        m_keepalive_retries = retries;
    }

    // Start listening for connections.
    bool server::listen(const socket_addresses& saddrs, const layer4_protocol& proto)
    {
        int domain = AF_INET; 
        int type = SOCK_STREAM;
        int fd = 0;
        int result = 0;
        bool err = false;
        for (auto it = saddrs.begin(); it != saddrs.end() ; ++it)
        {
            // Create socket and start listening.
            socket* sock = new socket(proto);
            sock->set_keepalive(m_keepalive);
            sock->set_keepalive_interval(m_keepalive_interval);
            sock->set_keepalive_retry(m_keepalive_retries);
            sock->set_keepalive_timeout(m_keepalive_timeout);
            // Listen on socket.
            if (sock->listen(*it, this) == true)
            {
                m_listen_sockets.lock();
                m_listen_sockets.get().push_back(sock);
                m_listen_sockets.unlock();
            }
            else
            {
                err = true;
            }
        }
        return !err;
    }

    // Return number of connected clients.
    int server::num_clients()
    {
        int result = 0;
        m_listen_sockets.lock();
        for (auto it = m_listen_sockets.get().begin(); 
            it != m_listen_sockets.get().end(); ++it)
        {
            socket* cur_sock = (*it);
            if (cur_sock == nullptr) continue;
            result += cur_sock->m_peers.get().size();
        }
        m_listen_sockets.unlock();
        return result; 
    }

    bool server::poll()
    {
        bool result = true;
        // Determine number of server-threads to spin up.
        int num_threads = m_listen_sockets.get().size();
        if (num_threads < config::get().get_server_min_threads())
        {
            num_threads = config::get().get_server_min_threads();
        }
        if (num_threads > config::get().get_server_max_threads())
        {
            num_threads = config::get().get_server_max_threads();
        }
        if (num_threads < m_listen_sockets.get().size())
            num_threads = m_listen_sockets.get().size();
        log::get().info("Starting "+std::to_string(num_threads)+" server threads.");
        std::vector<std::thread*> threads;
        sockets::iterator sock_it = m_listen_sockets.get().begin();
        for (int thread_id = 0; thread_id < num_threads; ++thread_id)
        {
            if (sock_it == m_listen_sockets.get().end())
            {
                sock_it = m_listen_sockets.get().begin();
            }
            socket* cur_sock = (*sock_it);
            if (cur_sock == nullptr) continue;
            threads.push_back(new std::thread(m_poll_single, cur_sock));
            sock_it++;
        }
        for (auto it = threads.begin(); it != threads.end(); ++it)
        {
            if ((*it) == nullptr) continue;
            (*it)->join();
            delete (*it);
            (*it) = nullptr;
        }
        return result;
    }

    // Events. ----------------------------------------------------------------

    void server::on_connect(peer* remote_peer)
    {
    }

    // Received data.
    void server::on_receive(peer* remote_peer)
    {
    }

    // Client disconnected.
    void server::on_disconnect(peer* remote_peer)
    {
    }

}

