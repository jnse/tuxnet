#include <iostream>
#include <unistd.h>
#include <tuxnet/tuxnet.h>

int main(int argc, char* argv[])
{
    tuxnet::server server;
    tuxnet::ip4_socket_address local_saddr(
        tuxnet::ip4_address("127.0.0.1"), 8080);
    tuxnet::socket_addresses saddrs;
    saddrs.push_back(&local_saddr);
    bool success = server.listen(
        const_cast<const tuxnet::socket_addresses&>(saddrs), 
        tuxnet::L4_PROTO_TCP);
    if (success != true)
    {
        return 1;
    }
    while(true)
    {
        if (server.poll() != true) break;
    }
    return 0;
}

