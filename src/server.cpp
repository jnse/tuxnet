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
        for (auto it = m_listen_sockets.begin(); 
            it != m_listen_sockets.end(); ++it)
        {
            delete (*it);
        }
        sockets().swap(m_listen_sockets);
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
                m_listen_sockets.push_back(sock);
            }
            else
            {
                err = true;
            }
        }
        return !err;
    }

    bool server::poll()
    {
        bool result = true;
        for (auto it = m_listen_sockets.begin(); 
            it != m_listen_sockets.end(); ++it)
        {
            /// @todo Each socket needs to be poll'ed in a separate thread in
            //        order to properly handle multiple listeners.
            socket* cur_sock = (*it);
            if (cur_sock == nullptr) continue;
            if (cur_sock->poll() != true)
            {
                result = false;
            }
        }
        return result;
    }

    // Events. ----------------------------------------------------------------

    // Client connected.
    void server::on_connect(peer* remote_peer)
    {
        log::get()->debug("Received connection.");
    }

    // Received data.
    void server::on_receive(peer* remote_peer)
    {
        log::get()->debug("Received data.");
    }

    // Client disconnected.
    void server::on_disconnect(peer* remote_peer)
    {
        log::get()->debug("Lost connection.");
    }

}

