/**
 * Implementation of IP address storage classes and
 * functions for IP address conversion.
 **/

#include <string.h>
#include <arpa/inet.h>
#include "tuxnet/ip_address.h"
#include "tuxnet/string.h"
#include "tuxnet/log.h"

namespace tuxnet
{
    /*************************************************************************
     * ip_address                                                            *
     *************************************************************************/

    // Constructor / destructor.-----------------------------------------------

    // Empty constructor.
    ip_address::ip_address() : m_proto(L3_PROTO_NONE)
    {

    }

    // Constructor with protocol constructor.
    ip_address::ip_address(layer3_protocol proto) : m_proto(proto)
    {

    }

    // Copy constructor.
    ip_address::ip_address(const ip_address& other)
    {

        m_proto = other.m_proto;
    }

    // Destructor.
    ip_address::~ip_address()
    {

    }

    // Setters. ---------------------------------------------------------------

    // Sets the layer3 protocol
    void ip_address::set_protocol(const layer3_protocol& proto)
    {
        m_proto = proto;
    }

    // Getters. ---------------------------------------------------------------

    const layer3_protocol& ip_address::get_protocol() const
    {
        return m_proto;
    }

    /*************************************************************************
     * ip4_address                                                           *
     *************************************************************************/

    // Constructors. ----------------------------------------------------------

    // Default/blank constructor.
    ip4_address::ip4_address() : m_addr({}), ip_address(L3_PROTO_IP4)
    {
    }

    // Copy constructor.
    ip4_address::ip4_address(const ip4_address& other) : 
        ip_address(L3_PROTO_IP4)
    {
        m_addr = other.m_addr;
    }

    // Constructor with digits.
    ip4_address::ip4_address(uint8_t a, uint8_t b, uint8_t c, uint8_t d) :
        ip_address(L3_PROTO_IP4)
    {
        set(a, b, c, d);
    }

    // Constructor with string.
    ip4_address::ip4_address(const std::string& ip_address) :
        tuxnet::ip_address(L3_PROTO_IP4)
    {
        set(ip_address);
    }

    // Constructor with in_addr.
    ip4_address::ip4_address(const in_addr& ip_address) : 
        tuxnet::ip_address(L3_PROTO_IP4)
    {
        m_addr = ip_address;
    }

    // Setters ----------------------------------------------------------------

    // Sets the IP address using individual digits.
    void ip4_address::set(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    {
        m_addr.s_addr = (d <<24 ) | (c << 16) | (b << 8) | a;
    }

    // Sets the IP address using a string.
    void ip4_address::set(const std::string& ip_address)
    {
        int errval = inet_pton(AF_INET, ip_address.c_str(), &m_addr);
        if (errval != 1)
        {
            std::string errstr = "The address ";
            errstr += ip_address;
            errstr += " is invalid, error ";
            errstr += std::to_string(errno);
            errstr += " (";
            errstr += strerror(errno);
            errstr += ")";
            log::get().error(errstr);
        }
    }

    // Sets the IP address using an in_addr struct.
    void ip4_address::set(const in_addr& ip_address)
    {
        m_addr = ip_address;
    }

    // Getters ----------------------------------------------------------------

    // Returns the IP address as string.
    std::string const ip4_address::as_string() const
    {
        return inet_ntoa(m_addr);
    }

    // Returns the IP address as in_addr.
    const in_addr& ip4_address::as_in_addr() const
    {
        return m_addr;
    }

    // Returns first digit of the IP address.
    uint8_t const ip4_address::get_a() const
    {
        return (m_addr.s_addr >> 24) & 0xFF; 
    }

    // Returns second digit of the IP address.
    uint8_t const ip4_address::get_b() const
    {
        return (m_addr.s_addr >> 16) & 0xFF;
    }

    // Returns third digit of the IP address.
    uint8_t const ip4_address::get_c() const
    {
        return (m_addr.s_addr >> 8) & 0xFF;
    }

    // Returns fourth digit of the IP address.
    uint8_t const ip4_address::get_d() const
    {
        return (m_addr.s_addr & 0xFF);
    }

}

