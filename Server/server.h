#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>

#include <QObject>
#include <QString>
#include <QtNetwork/QNetworkInterface>

#include <QDebug>

#include <vector>
#include <memory>
#include <optional>


class Server : public QObject
{
    Q_OBJECT // For the use of signals.

private: // Fields
    static constexpr unsigned short THREAD_NR   = 2;
    static constexpr unsigned short SERVER_PORT = 55555;

    std::unique_ptr<boost::asio::io_context>        io_cntxt;
    std::unique_ptr<boost::asio::io_context::work>  work;
    std::unique_ptr<boost::asio::ip::tcp::socket>   sckt;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;

    // It is passed by signal, therefore it must have a copy constructor
    std::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint;

    boost::thread_group threads;
    boost::mutex        mutex;

    std::vector<boost::uint8_t> received_buffer;
    std::vector<boost::uint8_t> send_buffer;
    std::size_t                 received_buffer_index;

    std::optional<std::atomic<bool>> serverStatus;  // true - is listening or is connected

private: // Methods
    void onAccept(const boost::system::error_code& ec)                    noexcept;
    void workerThread(const std::size_t index)                            noexcept;

    void onSend(const boost::system::error_code& ec, std::size_t n_bytes) noexcept;

    void recv()                                                           noexcept;
    void onRecv()                                                         noexcept;


signals:
    void listening_on(const std::shared_ptr<boost::asio::ip::tcp::endpoint>&);
    void connectionStatus(const char*);

public:
    Server(QObject* parent = nullptr);
    ~Server();

    const std::optional<std::atomic<bool>>& is_working() const;

    void listen()                                             noexcept;

    void send(const std::vector<boost::uint8_t>& send_buffer) noexcept;

    void closeConnection()                                    noexcept;

    void finish()                                             noexcept; // Server shutdown
};

#endif // SERVER_H
