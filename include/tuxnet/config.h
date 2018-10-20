#include <mutex>
#include <memory>

namespace tuxnet
{
    /**
     * The tuxnet configuration interface.
     */
    class config
    {

        // Private member variables. ------------------------------------------

        /// Maximum number of client threads.
        int m_client_max_threads;
        /// Minimum number of client threads.
        int m_client_min_threads;
        /// Holds singleton pointer to itself, instantiated on first use.
        static std::unique_ptr<config> m_instance;
        /// once_flag indicating if instance has already been allocated.
        static std::once_flag m_instance_allocated;
        /// Epoll event buffer size for listen sockets.
        int m_listen_socket_epoll_max_events;
        /// Epoll event buffer size for peer sockets.
        int m_peer_socket_epoll_max_events;
        /// Maximum number of server threads.
        int m_server_max_threads;
        /// Minimum number of server threads.
        int m_server_min_threads;

        public:

            // Ctor(s) / dtor. ------------------------------------------------

            /// Constructor.
            config();

            // Getters / setters. ---------------------------------------------

            /**
             * Get pointer to config instance.
             *
             * @return Returns a pointer to the config object singleton.
             *         Class is instantiated on first use.
             **/
            static config& get();

            /// Get maximum number of threads for communication with clients.
            int const get_client_max_threads();

            /// Get minimum number of threads for communication with clients.
            int const get_client_min_threads();

            /// Get epoll event buffer size for listen sockets.
            int const get_listen_socket_epoll_max_events();
            
            /// Get epoll event buffer size for peer sockets.
            int const get_peer_socket_epoll_max_events();

            /// Get maximum number of threads for accepting connections.
            int const get_server_max_threads();

            /// Get minimum number of threads for accepting connections.
            int const get_server_min_threads();


            /// @todo setters.
    
    };

}
