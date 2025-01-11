#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

#include <vector>

class Client : public boost::enable_shared_from_this<Client>
{
public:
    Client();
    ~Client() = default;
};

#endif // CLIENT_H
