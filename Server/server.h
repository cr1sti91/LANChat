#ifndef SERVER_H
#define SERVER_H

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
#include <QMenuBar>
#include <QObject>
#include <QLabel>
#include <QString>
#include <QtNetwork/QNetworkInterface>
#include <QDebug>

#include <cstring>
#include <vector>
#include <memory>
#include <functional>
#include <optional>


/**
 * @class Server
 * @brief A TCP server implementation using Boost.Asio and Qt for asynchronous operations and GUI integration.
 */
class Server : public QObject
{
    Q_OBJECT // For the use of signals.

private: // Fields
    static constexpr unsigned short THREAD_NR   = 2;            ///< Number of worker threads for Boost.Asio.
    static constexpr unsigned short SERVER_PORT = 55555;        ///< Default port number for the server.

    std::unique_ptr<boost::asio::io_context>        io_cntxt;   ///< Boost.Asio IO context.
    std::unique_ptr<boost::asio::io_context::work>  work;       ///< Keeps the io_context running.
    std::unique_ptr<boost::asio::ip::tcp::socket>   sckt;       ///< TCP socket for client connections.
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor;   ///< TCP acceptor for incoming connections.

    // It is passed by signal, therefore it must have a copy constructor
    std::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint;   ///< Server endpoint for binding and listening.

    boost::thread_group threads;                                ///< Worker threads for handling asynchronous operations.

    std::vector<boost::uint8_t> received_buffer;                ///< Buffer for storing received data.
    std::vector<boost::uint8_t> send_buffer;                    ///< Buffer for storing data to send.

    std::optional<std::atomic<bool>> serverStatus;              ///< Indicates whether the server is active or not.

private: // Methods
    /**
     * @brief Handles the completion of an asynchronous accept operation.
     * @param ec Error code resulting from the accept operation.
     */
    void onAccept(const boost::system::error_code& ec)    noexcept;
    /**
     * @brief Runs the Boost.Asio IO context in a separate worker thread.
     */
    void workerThread()                                   noexcept;
    /**
     * @brief Handles completion of a receive operation.
     * @param ec The error code from the operation.
     * @param bytes The number of bytes received.
     * @param displayMessage Function that displays the received message.
     *                       It receives as argument the type const std::string&.
     */
    void onRecv(const boost::system::error_code& ec, const size_t bytes,
                std::function<void(const std::string&)> displayMessage)    noexcept;


signals:
    /**
     * @brief Signal emitted when the server starts listening on an endpoint.
     * @param endpoint The endpoint on which the server is listening.
     */
    void listening_on(const std::shared_ptr<boost::asio::ip::tcp::endpoint>& endpoint);
    /**
     * @brief Signal emitted to indicate the server's connection status.
     * @param status Connection status message.
     */
    void connectionStatus(const char* status);


public:
    /**
     * @brief Constructs a new Server object.
     * @param parent The parent QObject.
     */
    Server(QObject* parent = nullptr);
    /**
     * @brief Destructor for the Server class.
     */
    ~Server();
    /**
     * @brief Gets the current status of the server.
     * @return Optional atomic boolean indicating if the server is active.
     *         If the listen method has not yet been called, the socket and
     *         acceptor do not require closing, and nullopt will be returned.
     */
    const std::optional<std::atomic<bool>>& is_working() const;
    /**
     * @brief Starts the server and listens for incoming connections.
     */
    void listen()                                                    noexcept;
    /**
     * @brief Sends data to the connected client.
     * @param send_buffer Buffer containing data to be sent.
     */
    void send(const std::vector<boost::uint8_t>& send_buffer)        noexcept;
    /**
     * @brief Starts receiving data from the client.
     * @param displayMessage Function that displays the received message.
     *                       It receives as argument the type const std::string&.
     */
    void recv(std::function<void(const std::string&)> displayMessage)noexcept;
    /**
     * @brief Closes the current client connection.
     */
    void closeConnection()                                           noexcept;
    /**
     * @brief Shuts down the server and stops all operations.
     */
    void finish()                                                    noexcept;
};

#endif // SERVER_H
