#ifndef PROTOCOL_H_INCLUDE
#define PROTOCOL_H_INCLUDE

namespace tuxnet
{

    /// Layer 3 protocol type.
    enum layer3_protocol
    {
        L3_PROTO_NONE = 0,
        L3_PROTO_IP4  = 1,
        L3_PROTO_IP6  = 2
    };

    /// Layer 4 protocol type.
    enum layer4_protocol
    {
        L4_PROTO_NONE = 0,
        L4_PROTO_TCP = 1,
        L4_PROTO_UDP = 2
    };

}

#endif

