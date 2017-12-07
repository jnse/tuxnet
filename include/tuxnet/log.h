#ifndef TUXNET_LOG_INCLUDE
#define TUXNET_LOG_INCLUDE

#include <string>

namespace tuxnet
{

    /// \TODO finish me.
    class log
    {

        /// Holds singleton pointer to itself, instantiated on first use.
        static log* m_this_ptr;
        
        public:
        
            static log* get_ptr();

            static void info(const std::string& message);
            static void debug(const std::string& message);

    };

}

#endif
