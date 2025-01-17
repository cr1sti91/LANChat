#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <QScrollArea>
#include <QMainWindow>
#include <QPalette>
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QObject>
#include <QLabel>
#include <QString>
#include <QtNetwork/QNetworkInterface>
#include <QDebug>

#include <cstring>
#include <vector>
#include <memory>
#include <optional>


class Client : public QObject
{
    Q_OBJECT // For the use of signals.

private: // Fields
    static constexpr unsigned short THREAD_NR   = 2;

    std::unique_ptr<boost::asio::io_context>        io_cntxt;
    std::unique_ptr<boost::asio::io_context::work>  work;
    std::unique_ptr<boost::asio::ip::tcp::socket>   sckt;

    // It is passed by signal, therefore it must have a copy constructor
    std::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint;

    boost::thread_group threads;
    boost::mutex        mutex;

    std::vector<boost::uint8_t> received_buffer;
    std::vector<boost::uint8_t> send_buffer;
    std::size_t                 received_buffer_index;

    std::optional<std::atomic<bool>> clientStatus;  // true - is connected

private: // Methods
    void onConnect(const boost::system::error_code& ec)                   noexcept;
    void workerThread()                                                   noexcept;

    void onSend(const boost::system::error_code& ec, std::size_t n_bytes) noexcept;

    void onRecv(const boost::system::error_code& ec, const size_t bytes, QLabel *messageLabel) noexcept;


signals:
    void connectionStatus(const char*);


public:
    Client(QObject* parent = nullptr);
    ~Client();

    const std::optional<std::atomic<bool>>& is_working() const;

    void connect(const char* ip_address, const unsigned port) noexcept;

    void send(const std::vector<boost::uint8_t>& send_buffer) noexcept;

    void recv(QLabel* messageLabel)                           noexcept;

    void closeConnection()                                    noexcept;

    void finish()                                             noexcept; // Client shutdown
};

#endif // CLIENT_H
