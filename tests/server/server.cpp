#include <iostream>
#include <unistd.h>
#include <tuxnet/tuxnet.h>

// Here's a test implementation of a server.
class my_server : public tuxnet::server
{
    public:

        // Constructor.
        my_server() : tuxnet::server()
        {
        }

    protected:

        // Runs when a client sends data.
        virtual void on_receive(tuxnet::peer* remote_peer)
        {
            std::string client_request = remote_peer->read_all();
            std::string response = "<h1>Hello world!</h1>";
            remote_peer->write_string("HTTP/1.1 200 OK\n");
            remote_peer->write_string("Connection: Close\n");
            remote_peer->write_string("Content-Encoding: text/plain\n");
            remote_peer->write_string("Content-Length: "
                + std::to_string(response.length()) + "\n");
            remote_peer->write_string("\r\n");
            remote_peer->write_string(response);
            remote_peer->disconnect();
        }

};

int main(int argc, char* argv[])
{
    // Instantiate server object.
    my_server server;
    // Define ip and port pairs to listen on.
    tuxnet::ip4_socket_address saddr_http(
        tuxnet::ip4_address("127.0.0.1"), 8080
    );
    tuxnet::ip4_socket_address saddr_https(
        tuxnet::ip4_address("127.0.0.1"), 8443
    );
    tuxnet::socket_addresses saddrs = { &saddr_http, &saddr_https };
    // Start listening.
    bool success = server.listen(saddrs, tuxnet::L4_PROTO_TCP);
    if (success != true)
    {
        return 1;
    }
    // Enter event loop.
    while(true)
    {
        if (server.poll() != true) break;
    }
    return 0;
}

