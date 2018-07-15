#include <string>
#include "tuxnet/log.h"
#include "tuxnet/protocol.h"

namespace tuxnet
{

    // Get a pointer to a protoent object from a layer-4 protocol.
    int layer4_to_proto(const layer4_protocol& proto)
    {
        protoent* p = nullptr;
        int pnum = proto;
        switch(pnum)
        {
            case L4_PROTO_TCP:
                p = getprotobyname("TCP");
                break;
            case L4_PROTO_UDP:
                p = getprotobyname("UDP");
                break;
            default:
                p = nullptr;
                break;
        }
        if (!p)
        {
            log::get().error(std::string("Could not get protocol ")
                + std::to_string(proto));
            return 0;
        }
        return p->p_proto;
    }

}
