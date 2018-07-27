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

        /// Holds singleton pointer to itself, instantiated on first use.
        static std::unique_ptr<config> m_instance;
        /// once_flag indicating if instance has already been allocated.
        static std::once_flag m_instance_allocated;
        /// Epoll event buffer size for listen sockets.
        int m_listen_socket_epoll_max_events;
        /// Epoll event buffer size for peer sockets.
        int m_peer_socket_epoll_max_events;
        
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
            
            /// Get epoll event buffer size for listen sockets.
            int const get_listen_socket_epoll_max_events();
            
            /// Get epoll event buffer size for peer sockets.
            int const get_peer_socket_epoll_max_events();
 
            /// @todo setters.
    
    };

}
