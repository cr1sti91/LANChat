#ifndef SERVER_H
#define SERVER_H

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <QDebug>
#include <QObject>
#include <QtNetwork/QNetworkInterface>

#include <cstring>
#include <vector>
#include <cstdint>
#include <memory>
#include <optional>


/**
 * @class Server
 * @brief A TCP server implementation using Boost.Asio and Qt for asynchronous operations and GUI integration.
 */
class Server : public QObject
{
    Q_OBJECT // For the use of signals.

    /**
     * @class Connection
     * @brief Represents an active TCP connection between the server and a client.
     *
     * This class manages the socket used for communication and
     * the connection state (active/inactive).
     */
    struct Connection
    {
        std::shared_ptr<boost::asio::ip::tcp::socket> socket; ///< Client socket.
        bool state;                                           ///< Socket status (connected or not)

        Connection(boost::asio::io_context& io_cntxt) :
            socket(std::make_shared<boost::asio::ip::tcp::socket>(io_cntxt)),
            state(false)
        {
        }
        ~Connection() = default;
    };

private: // Fields
    static constexpr std::uint8_t THREAD_NR      = 2;           ///< Number of worker threads for Boost.Asio.
    static constexpr unsigned     SERVER_PORT    = 55555;       ///< Default port number for the server.
    static constexpr std::uint8_t MAX_CLIENT_NUM = 5;           ///< Maximum number of clients that can connect.

    std::unique_ptr<boost::asio::io_context>        m_io_cntxt;   ///< Boost.Asio IO context.
    std::unique_ptr<boost::asio::io_context::work>  m_work;       ///< Keeps the io_context running.
    std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;   ///< TCP acceptor for incoming connections.

    std::vector<Connection*>                        m_connections;///< TCP client connections.

    // It is passed by signal, therefore it must have a copy constructor
    std::shared_ptr<boost::asio::ip::tcp::endpoint> m_endpoint;   ///< Server endpoint for binding and listening.

    boost::thread_group              m_threads;                   ///< Worker threads for handling asynchronous operations.

    std::vector<std::uint8_t>        m_received_buffer;           ///< Buffer for storing received data.
    std::vector<std::uint8_t>        m_send_buffer;               ///< Buffer for storing data to send.

    std::optional<std::atomic<bool>> m_serverStatus;              ///< Indicates whether the server is active or not.

    bool                             m_hasEverConnected;          ///< Indicates whether the server has had at least some
                                                                  ///< connections (or has).
    bool                             m_isGroupChat;               ///< If true, the message received from a client is automatically
                                                                  ///< sent to the rest of the active clients.

private: // Methods
    /**
     * @brief findLANIPAddress Obtaining the IP address of the device on the LAN.
     */
    void findLANIPAddress()                                noexcept;
    /**
     * @brief getSocketIndex
     * @return
     */
    std::optional<std::uint8_t> getSocketIndex()           noexcept;
    /**
     * @brief Listens for incoming connections.
     */
    void acceptConnection()                                noexcept;
    /**
     * @brief Handles the completion of an asynchronous accept operation.
     * @param ec Error code resulting from the accept operation.
     * @param socket_index The index of the socket
     */
    void onAccept(const boost::system::error_code& ec,
                  const std::uint8_t socket_index)          noexcept;
    /**
     * @brief Runs the Boost.Asio IO context in a separate worker thread.
     */
    void workerThread()                                     noexcept;
    /**
     * @brief Server::recv
     * @param socket_index
     */
    void recv(const std::uint8_t socket_index)              noexcept;
    /**
     * @brief Handles completion of a receive operation.
     * @param ec The error code from the operation.
     * @param bytes The number of bytes received.
     * @param socket_index The index of the socket for which the onRecv method calls
     *        the recv method
     */
    void onRecv(const boost::system::error_code& ec, const size_t bytes,
                const std::uint8_t socket_index)                            noexcept;
    /**
     * @brief onSend
     * @param ec
     * @param bytes
     */
    void onSend(const boost::system::error_code& ec, const size_t bytes)    noexcept;


signals:
    /**
     * @brief Signal emitted when the server starts listening on an endpoint.
     * @param endpoint The endpoint on which the server is listening.
     */
    void listening_on(const std::shared_ptr<boost::asio::ip::tcp::endpoint>& endpoint);
    /**
     * @brief message_received It is emitted when the server receives a message from the client.
     * @param message Message received
     */
    void message_received(const std::string& message);
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
     * @brief setGroupChat Setter for the m_isGroupChat field.
     * @param value New value.
     */
    void setGroupChat(const bool value)                              noexcept;
    /**
     * @brief getHasEverConnected
     * @return Returns a bool value indicating whether the server has had at
     *         least one connection (or has).
     */
    bool& getHasEverConnected()                                      noexcept;
    /**
     * @brief Gets the current status of the server.
     * @return Optional atomic boolean indicating if the server is active.
     *         If the listen method has not yet been called, the socket and
     *         acceptor do not require closing, and nullopt will be returned.
     */
    const std::optional<std::atomic<bool>>& is_working()       const noexcept;
    /**
     * @brief getClientNum
     * @return
     */
    uint8_t getClientNum()                                       const noexcept;
    /**
     * @brief startConnection Starts the server and listens for incoming connections.
     */
    void startConnection()                                           noexcept;
    /**
     * @brief Sends data to the connected client.
     * @param send_buffer Buffer containing data to be sent.
     * @param socket_index If the socket index is not specified, the
     *        message is sent to all active clients.
     */
    void send(const std::vector<std::uint8_t>& send_buffer,
              std::optional<std::uint8_t> socket_index = std::nullopt)noexcept;
    /**
     * @brief Starts receiving data from the client.
     */
    void startRecv()                                                 noexcept;
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
