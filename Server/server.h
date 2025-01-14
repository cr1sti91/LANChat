#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>

#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkInterface>

#include <vector>


class Server : public QObject
{
    Q_OBJECT // For the use of signals.

private: // Fields
    static constexpr unsigned short THREAD_NR = 2;
    static constexpr  unsigned short SERVER_PORT = 55555;

    boost::shared_ptr<boost::asio::io_context>        io_cntxt;
    boost::shared_ptr<boost::asio::io_context::work>  work;
    boost::shared_ptr<boost::asio::ip::tcp::socket>   sckt;
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
    boost::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint;

    boost::thread_group threads;
    boost::mutex        mutex;

    std::vector<boost::uint8_t> received_buffer;
    std::vector<boost::uint8_t> send_buffer;
    std::size_t                 received_buffer_index;
    boost::atomic<bool>         end;


private: // Methods
    void onAccept(const boost::system::error_code& ec) noexcept;
    void workerThread(const std::size_t index);

    void onSend(const boost::system::error_code& ec, std::size_t n_bytes) noexcept;

    void recv() noexcept;
    void onRecv() noexcept;


signals:
    void listening_on(boost::shared_ptr<boost::asio::ip::tcp::endpoint>);
    void exception_workerThread();
    void connectionStatus(const char*);
    void error_workerThread();
    void onSend_error();

public:
    Server();
    ~Server() = default;

    void listen() noexcept;

    void send(const std::vector<boost::uint8_t>& send_buffer) noexcept;

    void finish() noexcept; // Server shutdown
};

#endif // SERVER_H
