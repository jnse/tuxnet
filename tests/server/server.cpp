#include <unistd.h>
#include <tuxnet/tuxnet.h>

int main(int argc, char* argv[])
{
    tuxnet::server s;
    tuxnet::ip4_socket_address local_saddr(
        tuxnet::ip4_address("127.0.0.1"), 
        8080);
    //s.listen(tuxnet::socket_addresses({"127.0.0.1:8000"}));
    //while(true){ sleep(1); }
    return 0;
}

