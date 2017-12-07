#include "tuxnet/log.h"

namespace tuxnet
{

    /// \TODO finish me.
    log* log::get_ptr()
    {
        if (m_this_ptr == nullptr) m_this_ptr = new log();
        return m_this_ptr;
    }

    void log::info(const std::string& message)
    {
    }

    void log::debug(const std::string& message)
    {
    }

}
