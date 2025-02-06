#include "server.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
void Server::findLANIPAddress() noexcept
{
    try
    {
        // Finding the LAN IP address on Linux/Windows (wi-fi or ethernet)
        const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

        for (const QNetworkInterface &iface : interfaces)
        {
            const QString ifaceName = iface.name();
            boost::asio::ip::address private_IP_address;
            bool wifi_or_ethernet = false;

        // Checking interfaces depending on the operating system.
#ifdef __linux__
            wifi_or_ethernet = ifaceName.startsWith("wl") ||
                               ifaceName.startsWith("en") ||
                               ifaceName.startsWith("eth");
#elif _WIN32
            wifi_or_ethernet = ifaceName.contains("Wi-Fi", Qt::CaseInsensitive)) ||
                               ifaceName.startsWith("Ethernet", Qt::CaseInsensitive);
#else
            emit this->connectionStatus("Unknown OS!");
            return;
#endif
            if (wifi_or_ethernet)
            {
                const QList<QNetworkAddressEntry> entries = iface.addressEntries();
                for (const QNetworkAddressEntry &entry : entries)
                {
                    // If the address is not local to the device only, but is part of the LAN,
                    // it can be used by the server for listening.
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.ip().isLoopback())
                    {
                        private_IP_address = boost::asio::ip::make_address(entry.ip().toString().toStdString());
                        m_endpoint = std::make_shared<boost::asio::ip::tcp::endpoint>(private_IP_address,
                                                                                      SERVER_PORT);
                        break;
                    }
                }

                if(m_endpoint)
                    break;
            }
        }
    }
    catch(const std::exception& e)
    {
        emit this->connectionStatus(e.what());
    }
}

std::optional<uint8_t> Server::getSocketIndex() noexcept
{
    if(!m_connections.empty())
    {
        for(uint8_t i = 0; i < m_connections.size(); ++i)
            if(!m_connections.at(i)->state)
                return i;
    }

    if(m_connections.size() < MAX_CLIENT_NUM)
    {
        m_connections.push_back(new Connection(*m_io_cntxt));
        return uint8_t(m_connections.size() - 1);
    }

    return std::nullopt;
}


void Server::acceptConnection() noexcept
{
    try
    {
        const std::optional<std::uint8_t> socket_index = getSocketIndex();
        if(socket_index.has_value())
        {
            m_acceptor->async_accept(*m_connections.at(socket_index.value())->socket,
                                     boost::bind(&Server::onAccept,
                                                 this,
                                                 boost::asio::placeholders::error,
                                                 socket_index.value()
                                                 )
                                     );

            emit this->listening_on(m_endpoint);
        }
        else
        {
            emit this->connectionStatus("No more clients can connect to the server.");
        }
    }
    catch (const std::exception& e)
    {
        emit connectionStatus(e.what());
    }
}


void Server::onAccept(const boost::system::error_code &ec, const std::uint8_t socket_index) noexcept
{
    if(ec)
    {
        if(m_serverStatus.has_value() && m_serverStatus.value())
            emit this->connectionStatus("  Connection failed!");
    }
    else
    {
        // The socket is connected and the async_read method can be called for it.
        m_connections.at(socket_index)->state = true;

        emit this->connectionStatus("  Connected!");

        // If the number of clients connected to the server exceeds the maximum
        // number, the acceptor closes
        if(!(m_connections.size() < MAX_CLIENT_NUM))
            m_acceptor->close();
        else
            this->acceptConnection();
    }
}

void Server::workerThread() noexcept
{
    while(true)
    {
        try
        {
            boost::system::error_code ec;
            m_io_cntxt->run(ec);

            if(ec)
            {
                emit this->connectionStatus("Error m_workerThread");
            }
            break;
        }
        catch(const std::exception& e)
        {
            emit this->connectionStatus("Exception m_workerThread");
        }
    }
}

void Server::recv(const std::uint8_t socket_index) noexcept
{
    try
    {
        if(m_serverStatus.has_value() && m_serverStatus.value())
        {
            m_connections.at(socket_index)->socket->async_read_some(
                boost::asio::buffer(m_received_buffer, m_received_buffer.size()),
                boost::bind(&Server::onRecv,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred,
                            socket_index
                            )
            );
        }
    }
    catch (const std::exception& e)
    {
        if(m_serverStatus.has_value() && m_serverStatus.value())
            emit this->connectionStatus(e.what());
    }
}

void Server::onRecv(const boost::system::error_code& ec, const size_t bytes,
                    const std::uint8_t socket_index)                                  noexcept
{
    if(ec)
    {
        if(m_serverStatus.has_value() && m_serverStatus.value())
            emit this->connectionStatus("Async_read_some error!");

        return;
    }

    std::string received_message(m_received_buffer.begin(), m_received_buffer.begin() + bytes);

    emit message_received(received_message);

    // If m_isGroupChat is true, the message received from a client is automatically sent to
    // the rest of the active clients.
    if(m_isGroupChat)
    {
        std::vector<boost::uint8_t> echo_buffer(m_received_buffer.begin(),
                                                m_received_buffer.begin() + bytes);
        for(std::uint8_t i = 0; i < m_connections.size(); ++i)
        {
            if(i == socket_index)
                continue;
            else
                this->send(echo_buffer, i);
        }
    }

    this->recv(socket_index);
}

void Server::onSend(const boost::system::error_code& ec, const size_t bytes) noexcept
{
    if(ec && m_serverStatus.has_value() && m_serverStatus.value())
    {
        emit this->connectionStatus("An error occurred while transmitting data.");
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS
///
Server::Server(QObject* parent)
    : QObject(parent),
      m_endpoint(nullptr),
      m_serverStatus(std::nullopt),
      m_hasEverConnected(false),
      m_isGroupChat(false)
{
    // Initialization
    m_io_cntxt   = std::make_unique<boost::asio::io_context>();
    m_work       = std::make_unique<boost::asio::io_service::work>(*m_io_cntxt);
    m_acceptor   = std::make_unique<boost::asio::ip::tcp::acceptor>(*m_io_cntxt);

    // Creating threads for receiving and sending data
    for(std::uint8_t i = 0; i < THREAD_NR; ++i)
        m_threads.create_thread(boost::bind(&Server::workerThread, this));

    m_received_buffer.resize(4096);
}


Server::~Server()
{
    if(m_serverStatus.has_value() && m_serverStatus.value())
        this->finish();

    m_threads.join_all();

    for(auto& connection : m_connections)
        delete connection;
}

void Server::setGroupChat(const bool value) noexcept
{
    m_isGroupChat = value;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS (STATUS GETTERS)
///
bool &Server::getHasEverConnected() noexcept
{
    return m_hasEverConnected;
}

const std::optional<std::atomic<bool>> &Server::is_working() const noexcept
{
    return m_serverStatus;
}

std::uint8_t Server::getClientNum() const noexcept
{
    std::uint8_t num = 0;
    for(const auto& connection : m_connections)
        if(connection->state) ++num;

    return num;
}

void Server::startConnection() noexcept
{
    m_serverStatus = true;

    m_hasEverConnected = true;

    if(!m_endpoint)
        this->findLANIPAddress();

    try
    {
        if(m_endpoint)
        {
            // If there is a valid endpoint, m_acceptor start listening
            boost::system::error_code ec;

            m_acceptor->open(m_endpoint->protocol());
            m_acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            m_acceptor->bind(*m_endpoint, ec);

            if(ec)
            {
                throw std::runtime_error("Invalid m_endpoint (probably the "
                                         "port is occupied by another instance)!");

                m_endpoint.reset();
                m_endpoint = nullptr;
                return;
            }

            m_acceptor->listen(MAX_CLIENT_NUM);
        }
        else
        {
            throw std::runtime_error("No valid m_endpoint found after scanning interfaces!");
        }

        this->acceptConnection();
    }
    catch (const std::exception& e)
    {
        emit connectionStatus(e.what());
    }
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void Server::send(const std::vector<std::uint8_t>& send_buffer,
                  std::optional<std::uint8_t> socket_index)                  noexcept
{
    try
    {
        if(socket_index.has_value())
        {
            auto& connection = m_connections.at(socket_index.value());

            if(connection->state)
            {
                boost::asio::async_write(*connection->socket,
                                         boost::asio::buffer(send_buffer),
                                         boost::bind(&Server::onSend,
                                                     this,
                                                     boost::asio::placeholders::error,
                                                     boost::asio::placeholders::bytes_transferred
                                                     )
                                         );
            }
        }
        else
        {
            for(auto& connection : m_connections)
            {
                // If the socket is connected, then writing to it is allowed.
                if(connection->state)
                {
                    boost::asio::async_write(*connection->socket,
                                             boost::asio::buffer(send_buffer),
                                             boost::bind(&Server::onSend,
                                                         this,
                                                         boost::asio::placeholders::error,
                                                         boost::asio::placeholders::bytes_transferred
                                                         )
                                             );
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        emit this->connectionStatus(e.what());
    }
}

void Server::startRecv() noexcept
{
    for(std::uint8_t i = 0; i < m_connections.size(); ++i)
    {
        if(m_connections.at(i)->state)
            this->recv(i);
    }
}


void Server::closeConnection() noexcept
{
    m_serverStatus = false;

    boost::system::error_code ec;

    try
    {
        // When the server starts a new connection session, all current connections are closed.
        for(auto& connection : m_connections)
        {
            if(connection->state)
            {
                connection->state = false;

                if(connection->socket->is_open())
                {
                    connection->socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                    connection->socket->close(ec);

                    if(ec) emit connectionStatus("Error on socket shutdown/close!");
                }
            }
        }

        if(m_acceptor->is_open())
            m_acceptor->close();
    }
    catch (const std::exception& e)
    {
        emit this->connectionStatus(e.what());
    }
}


void Server::finish() noexcept
{
    this->closeConnection();

    m_work.reset();
    m_io_cntxt->stop();
    m_threads.join_all();
}
