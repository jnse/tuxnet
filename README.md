What is it
----------

This is an as-simple-as-possible c++ linux-only network library with focus on
being able to process as much I/O and rate of incomming connections as possible.

Dependencies
------------
You won't need much other than a Linux system and a decently recent gcc
compiler.

* gcc 7 or greater (for C++17 support).
* Linux (probably at least >2.6.8 but you probably want 3.x or 4.x at this
  point anyway).
* pthread (If you have a Linux system you probably already have it).

No support for non-linux systems as of yet, due to everything being pure
epoll based for now.

Documentation
-------------

API documentation can be found here : https://jnse.github.io/tuxnet/html/

Example usage
-------------

Have a look at : 
https://github.com/jnse/tuxnet/blob/master/tests/server/server.cpp

State
-----

Still very much a work in progress. Barebones server implementation is there,
IPv4 and TCP only so far. No client implementation yet.

