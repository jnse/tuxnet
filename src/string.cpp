#include "tuxnet/string.h"

namespace tuxnet
{

    // Splits a string using another string as delimiter.
    str_vector str_split(const std::string& str, 
        const std::string& delim, int max_elem)
    {
        size_t pos=0;
        std::string s = str;
        str_vector result;
        int tokens = 1;
        while (((pos = s.find(delim)) != std::string::npos)
            and ((max_elem < 0) or (tokens != max_elem)))
        {
            result.push_back(s.substr(0,pos));
            s.erase(0, pos + delim.length());
            tokens++;
        }
        if (max_elem != 0) result.push_back(s);
        return result;
    }

}

