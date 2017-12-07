
#include <tuxnet/tuxnet.h>

int main(int argc, char* argv[])
{
    tuxnet::server s;
    s.listen(tuxnet::str_vector({"127.0.0.1:80"}));
    return 0;
}

