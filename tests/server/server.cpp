#include <unistd.h>
#include <tuxnet/tuxnet.h>

int main(int argc, char* argv[])
{
    tuxnet::server s;
    s.listen(tuxnet::str_vector({"127.0.0.1:8000"}));
    while(true){ sleep(1); }
    return 0;
}

