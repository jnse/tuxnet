#ifndef TUXNET_STRING_H_INCLUDE
#define TUXNET_STRING_H_INCLUDE

#include <string>
#include <vector>

namespace tuxnet
{

    /// Convenience typedef for a vector of strings.
    typedef std::vector<std::string> str_vector;

    /** 
     * Splits a string using another string as delimiter.
     *
     * @param str : String to split.
     * @param delim : Delimiter to use.
     * @param max_elem : (optional) Split string up to maximum max_elem times.
     * @return str_vector : Returns a vector of strings containing string str
     *                      substrings split up by delim.
     **/
    str_vector str_split(const std::string& str, 
        const std::string& delim, int max_elem=-1);
}

#endif
