#include <iostream>
#include "tuxnet/log.h"

namespace tuxnet
{

    // Initialize log instance pointer.
    log* log::m_this_ptr = 0;

    // Get pointer to log instance.
    log* log::get()
    {
        if (m_this_ptr == nullptr) m_this_ptr = new log();
        return m_this_ptr;
    }

    //  Log an informational message.
    void log::info(const std::string& message)
    {
        std::cout << message << std::endl;
    }

    // Log a debug message.
    void log::debug(const std::string& message)
    {
        std::cout << message << std::endl;
    }

    // Log an error message.
    void log::error(const std::string& message)
    {
        std::cerr << message << std::endl;
    }


}
