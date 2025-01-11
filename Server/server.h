#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>

#include <QObject>
#include <QString>
#include <QLineEdit>

#include <vector>


class Server : public QObject
{
    Q_OBJECT // For the use of signals.

private:
    static constexpr short thread_nr = 2;

    boost::shared_ptr<boost::asio::io_context> io_cntxt;
    boost::shared_ptr<boost::asio::io_context::work> work;
    boost::shared_ptr<boost::asio::ip::tcp::socket> sckt;
    boost::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;

    boost::thread_group threads;
    boost::mutex mutex;

    std::vector<boost::uint8_t> received_buffer;
    std::vector<boost::uint8_t> send_buffer;
    std::size_t received_buffer_index;
    boost::atomic<bool> end;


signals:
    void listening_on();
    void exception_listening_on();
    void exception_workerThread();
    void connection_failed();
    void connected();
    void error_workerThread();
    void onSend_error();

public:
    Server();
    ~Server() = default;

    void listen(const char* address, const std::size_t port, const QLineEdit* const lineEdit) noexcept;
    void onAccept(const boost::system::error_code& ec, const QLineEdit* const lineEdit) noexcept;
    void workerThread(const std::size_t index);

    void send(const QLineEdit* const lineEdit) noexcept;
    void onSend(const boost::system::error_code& ec, std::size_t n_bytes, const QLineEdit* const lineEdit) noexcept;

    void recv() noexcept;
    void onRecv() noexcept;

    void finish() noexcept; // Server shutdown
};

#endif // SERVER_H
