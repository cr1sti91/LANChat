#include "server.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
void Server::getLANIPAddress() noexcept
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
                else
                    throw std::runtime_error("No valid m_endpoint found after scanning interfaces!");
            }
        }


        // If there is a valid endpoint, m_acceptor start listening
        boost::system::error_code ec;

        m_acceptor->open(m_endpoint->protocol());
        m_acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor->bind(*m_endpoint, ec);

        if(ec)
        {
            emit this->connectionStatus("Invalid m_endpoint (probably the "
                                        "port is occupied by another instance)!");
            m_endpoint.reset();
            m_endpoint = nullptr;
            return;
        }

        m_acceptor->listen(boost::asio::socket_base::max_connections);
    }
    catch(const std::exception& e)
    {
        emit this->connectionStatus(e.what());
    }
}


void Server::onAccept(const boost::system::error_code &ec) noexcept
{
    if(ec)
    {
        emit this->connectionStatus("  Connection failed!");
    }
    else
    {
        emit this->connectionStatus("  Connected!");

        if(!(m_client_sockets.size() < MAX_CLIENT_NUM))
            m_acceptor->close();
        else
        {
            this->recv();
            this->listen();
        }
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

void Server::onRecv(const boost::system::error_code& ec, const size_t bytes) noexcept
{
    if(ec)
    {
        if(m_serverStatus.has_value() && m_serverStatus.value())
            emit this->connectionStatus("Async_read_some error!");

        return;
    }

    std::string received_message(m_received_buffer.begin(), m_received_buffer.begin() + bytes);

    emit message_received(received_message);

    this->recv();
}

void Server::onSend(const boost::system::error_code &ec, const size_t bytes) noexcept
{
    if(ec)
    {
        emit this->connectionStatus("An error occurred while transmitting data.");
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS
///
Server::Server(QObject* parent) : QObject(parent),
                                  m_endpoint(nullptr),
                                  m_serverStatus(std::nullopt)
{
    // Initialization
    m_io_cntxt   = std::make_unique<boost::asio::io_context>();
    m_work       = std::make_unique<boost::asio::io_service::work>(*m_io_cntxt);
    m_acceptor   = std::make_unique<boost::asio::ip::tcp::acceptor>(*m_io_cntxt);

    // Creating threads for receiving and sending data
    for(short i = 0; i < THREAD_NR; ++i)
        m_threads.create_thread(boost::bind(&Server::workerThread, this));

    m_received_buffer.resize(4096);
}


Server::~Server()
{
    if(m_serverStatus.has_value() && m_serverStatus.value())
        this->finish();

    m_threads.join_all();
}

const std::optional<std::atomic<bool>> &Server::is_working() const
{
    return m_serverStatus;
}

void Server::listen() noexcept
{
    m_serverStatus = true;

    if(!m_endpoint)
        this->getLANIPAddress();

    try
    {        
        // Initializing a socket for a connection (false means that it's not connected)
        m_client_sockets.push_back(
            {std::make_shared<boost::asio::ip::tcp::socket>(*m_io_cntxt), false}
        );

        m_acceptor->async_accept(*m_client_sockets.at(m_client_sockets.size() - 1).first,
                                 boost::bind(&Server::onAccept,
                                             this,
                                             boost::asio::placeholders::error
                                             )
                                 );

        emit this->listening_on(m_endpoint);
    }
    catch (const std::exception& e)
    {
        emit this->connectionStatus(e.what());
    }
}


void Server::send(const std::vector<boost::uint8_t>& send_buffer) noexcept
{
    try
    {
        for(auto& [socket, state] : m_client_sockets)
        {
            boost::asio::async_write(*socket,
                                     boost::asio::buffer(send_buffer),
                                     boost::bind(&Server::onSend,
                                                 this,
                                                 boost::asio::placeholders::error,
                                                 boost::asio::placeholders::bytes_transferred
                                                 )
                                      );
        }
    }
    catch (const std::exception& e)
    {
        emit this->connectionStatus(e.what());
    }
}


void Server::recv() noexcept
{
    try
    {
        if(m_serverStatus.has_value() && m_serverStatus.value())
        {
            m_client_sockets.at(m_client_sockets.size() - 1).first->async_read_some(
                boost::asio::buffer(m_received_buffer, m_received_buffer.size()),
                boost::bind(&Server::onRecv,
                            this,
                            boost::asio::placeholders::error,
                            boost::asio::placeholders::bytes_transferred
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


void Server::closeConnection() noexcept
{
    m_serverStatus = false;

    boost::system::error_code ec;

    try
    {
        // When the server starts a new connection session, all current connections are closed.b
        for(auto& [socket, state] : m_client_sockets)
        {
            if(socket->is_open())
            {
                socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
                socket->close(ec);

                if(ec) emit connectionStatus("Error on socket shutdown/close!");
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
    m_serverStatus = false;

    this->closeConnection();

    m_work.reset();
    m_io_cntxt->stop();
    m_threads.join_all();
}
