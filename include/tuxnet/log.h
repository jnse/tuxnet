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

            /**
             * Get pointer to log instance.
             *
             * @return Returns a pointer to the log object singleton.
             *         Class is instantiated on first use.
             **/
            static log* get();

            /**
             * Log an informational message.
             *
             * @param message : Message to be logged.
             **/
            void info(const std::string& message);

            /**
             * Log a debug message.
             *
             * @param message : Message to be logged.
             **/
            void debug(const std::string& message);

            /**
             * Log an error message.
             *
             * @param message : Message to be logged.
             **/
            void error(const std::string& message);


    };

}

#endif
