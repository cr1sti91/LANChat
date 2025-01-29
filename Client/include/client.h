#ifndef CLIENT_H
#define CLIENT_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <QScrollArea>
#include <QScrollBar>
#include <QMainWindow>
#include <QPalette>
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QResizeEvent>
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

/**
 * @class Client
 * @brief This class manages a TCP client for communication with a server.
 *        It provides methods for connecting to the server, sending/receiving messages,
 *        and managing the client lifecycle.
 */
class Client : public QObject
{
    Q_OBJECT // For the use of signals.

private: // Fields
    static constexpr unsigned short THREAD_NR   = 2;              ///< Number of worker threads.

    std::unique_ptr<boost::asio::io_context>        m_io_cntxt;   ///< IO context for asynchronous operations.
    std::unique_ptr<boost::asio::io_context::work>  m_work;       ///< Keeps the IO context alive.
    std::unique_ptr<boost::asio::ip::tcp::socket>   m_sckt;       ///< TCP socket for communication.

    // It is passed by signal, therefore it must have a copy constructor
    std::shared_ptr<boost::asio::ip::tcp::endpoint> m_endpoint;   ///< Server endpoint.

    boost::thread_group              m_threads;                   ///< Thread group for worker threads.

    std::vector<boost::uint8_t>      m_received_buffer;           ///< Buffer for received data.
    std::vector<boost::uint8_t>      m_send_buffer;               ///< Buffer for data to send.

    std::optional<std::atomic<bool>> m_clientStatus;              ///< Indicates if the client is connected.

private:
    /**
     * @brief Handles connection result.
     * @param ec The error code resulting from the connection attempt.
     */
    void onConnect(const boost::system::error_code& ec)                   noexcept;
    /**
     * @brief Executes worker threads to process IO context tasks.
     */
    void workerThread()                                                   noexcept;
    /**
     * @brief Handles completion of a send operation.
     * @param ec The error code from the operation.
     * @param n_bytes The number of bytes sent.
     */
    void onSend(const boost::system::error_code& ec, std::size_t n_bytes) noexcept;
    /**
     * @brief Handles completion of a receive operation.
     * @param ec The error code from the operation.
     * @param bytes The number of bytes received.
     */
    void onRecv(const boost::system::error_code& ec, const size_t bytes)  noexcept;


signals:
    /**
     * @brief message_received It is emitted when the client receives a message from the server.
     * @param message Message received
     */
    void message_received(const std::string& message);
    /**
     * @brief Emitted to inform about the connection status.
     * @param status The connection status message.
     */
    void connectionStatus(const char* status);


public:
    /**
     * @brief Constructs a Client instance.
     * @param parent The parent QObject.
     */
    Client(QObject* parent = nullptr);
    /**
     * @brief Destructor for the Client.
     */
    ~Client();
    /**
     * @brief Checks if the client is working.
     * @return An optional boolean indicating the client status.
     */
    const std::optional<std::atomic<bool>>& is_working() const       noexcept;
    /**
     * @brief Initiates a connection to a server.
     * @param ip_address The server IP address.
     * @param port The server port.
     */
    void connect(const char* ip_address, const unsigned port)        noexcept;
    /**
     * @brief Sends data to the server.
     * @param send_buffer The data buffer to send.
     */
    void send(const std::vector<boost::uint8_t>& send_buffer)        noexcept;
    /**
     * @brief Starts receiving data from the server.
     */
    void recv() noexcept;
    /**
     * @brief Closes the connection to the server.
     */
    void closeConnection()                                           noexcept;
    /**
     * @brief Shuts down the server and stops all operations.
     */
    void finish()                                                    noexcept;
};

#endif // CLIENT_H
