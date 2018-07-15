#ifndef TUXNET_LOG_INCLUDE
#define TUXNET_LOG_INCLUDE

#include <memory>
#include <mutex>
#include <string>

namespace tuxnet
{

    /**
     * The tuxnet logging interface.
     * @todo finish me.
     */
    class log
    {

        /// Holds singleton pointer to itself, instantiated on first use.
        static std::unique_ptr<log> m_instance;
        /// once_flag indicating if instance has already been allocated.
        static std::once_flag m_instance_allocated;

        public:

            /**
             * Get pointer to log instance.
             *
             * @return Returns a pointer to the log object singleton.
             *         Class is instantiated on first use.
             **/
            static log& get();

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
