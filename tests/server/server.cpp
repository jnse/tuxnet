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

        // Destructor.
        ~my_server()
        {
        }

    protected:

        // Runs when a client connects.
        virtual void on_connect(tuxnet::peer* remote_peer)
        {
            std::cout << "Received connection (clients = ";
            std::cout << std::to_string(num_clients());
            std::cout << ")." << std::endl;
        }

        // Runs when a client sends data.
        virtual void on_receive(tuxnet::peer* remote_peer)
        {
            std::string text = remote_peer->read_line();
            if (text != "")
            {
                if (text == "GET / HTTP/1.1")
                {
                    std::string response = "hello!";
                    remote_peer->write_string("HTTP/1.1 200 OK\n");
                    remote_peer->write_string("Connection: Closed\n");
                    remote_peer->write_string("Content-Encoding: text/plain\n");
                    remote_peer->write_string("Content-Length: "
                        + std::to_string(response.length()) + "\n");
                    remote_peer->write_string("\r\n");
                    remote_peer->write_string(response);
                    remote_peer->disconnect();
                }
            }
        }

        // Runs when a client disconnects.
        virtual void on_disconnect(tuxnet::peer* remote_peer)
        {
            std::cout << "Client disconnected (clients = ";
            std::cout << std::to_string(num_clients()-1);
            std::cout << ")." << std::endl;
        }
};

int main(int argc, char* argv[])
{
    std::cout.setf(std::ios::unitbuf);
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

