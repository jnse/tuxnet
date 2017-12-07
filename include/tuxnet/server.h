#ifndef SERVER_H_INCLUDE
#define SERVER_H_INCLUDE

#include "tuxnet/string.h"

namespace tuxnet
{
    class server
    {

        public:

            /**
             * Constructor.
             */
            server();

            /**
             * Destructor.
             */
            virtual ~server();

            /**
             * Start listening for connections.
             *
             * @param listen_addresses array with address:port pairs.
             */
            virtual void listen(str_vector listen_addresses);
    };
}

#endif
