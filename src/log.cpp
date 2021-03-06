#include <mutex>
#include <iostream>
#include <stdlib.h>
#include "tuxnet/log.h"

namespace tuxnet
{

    /// Init log class static members.
    std::unique_ptr<log> log::m_instance;
    std::once_flag log::m_instance_allocated;
    std::mutex m_lock;

    // Get pointer to log instance.
    log& log::get()
    {
        std::call_once(m_instance_allocated,[]{
            m_instance.reset(new log);
            std::cout.setf(std::ios::unitbuf);
        });
        return *m_instance.get();
    }

    //  Log an informational message.
    void log::info(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        std::cout << message << std::endl;
    }

    // Log a debug message.
    void log::debug(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        std::cout << message << std::endl;
    }

    // Log an error message.
    void log::error(const std::string& message)
    {
        std::lock_guard<std::mutex> lock(m_lock);
        std::cerr << message << std::endl;
        exit(1);
    }


}
