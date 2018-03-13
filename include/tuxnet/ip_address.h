/**
 * Class headers for IP address storage classes, 
 * and function definitions for IP address conversion.
 **/

#ifndef IP_ADDRESS_H_INCLUDE
#define IP_ADDRESS_H_INCLUDE

#include <cstdint>
#include <netinet/in.h>
#include <string>

namespace tuxnet
{

    /**
     * Stores an IPv4 address.
     *
     * This class stores an IPv4 address and has various 
     * getters/setters for compatibility with various commonly
     * used IP storage formats.
     *
     * Internally this class stores the IP as an in_addr struct,
     * thus if speed is a concern, the in_addr getter/setters
     * should be used. The other getter/setters convert to/from
     * in_addr.
     */
    class ip4_address
    {

        // Private member variables. ------------------------------------------
    
        /// Stores IP.
        in_addr m_addr;

        public:

            // Constructors. --------------------------------------------------        

            /// Constructor.
            ip4_address();

            /// Copy constructor.
            ip4_address(const ip4_address& other);

            /**
             * Constructor.
             *
             * @param a : First digit of the IP address.
             * @param b : Second digit of the IP address.
             * @param c : Third digit of the IP address.
             * @param d : Fourth digit of the IP address.
             */
            ip4_address(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

            /**
             * Constructor.
             *
             * @param ip_address IP address as string.
             */
            ip4_address(const std::string& ip_address); 

            /**
             * Constructor.
             *
             * @param ip_address IP address as in_addr structure.
             */
            ip4_address(const in_addr& ip_address);

            // Setters. -------------------------------------------------------

            /**
             * Sets the IP address.
             *
             * @param a : First digit of the IP address.
             * @param b : Second digit of the IP address.
             * @param c : Third digit of the IP address.
             * @param d : Fourth digit of the IP address.
             */
            void set(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

            /**
             * Sets the IP address.
             *
             * @param ip_address IP address as string.
             */
            void set(const std::string& ip_address);

            /**
             * Sets the IP address.
             *
             * @param ip_address IP address as in_addr structure.
             */
            void set(const in_addr& ip_address);

            /**
             * Gets the IP address.
             *
             * @return Returns the IP address as string.
             */
            std::string const as_string() const;

            /**
             * Gets the IP address.
             *
             * @return Returns the IP address as in_addr structure.
             */
            const in_addr& as_in_addr() const;

            /// Gets first digit of the IP address.
            uint8_t const get_a() const;

            /// Gets second digit of the IP address.
            uint8_t const get_b() const;

            /// Gets third digit of the IP address.
            uint8_t const get_c() const;

            /// Gets fourth digit of the IP address.
            uint8_t const get_d() const;

    };
}

#endif

