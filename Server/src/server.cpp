#include "server.h"

Server::Server(QObject* parent) : QObject(parent),
                                  m_endpoint(nullptr),
                                  m_serverStatus(std::nullopt)
{
    // Initialization
    m_io_cntxt   = std::make_unique<boost::asio::io_context>();
    m_work       = std::make_unique<boost::asio::io_service::work>(*m_io_cntxt);
    m_sckt       = std::make_unique<boost::asio::ip::tcp::socket>(*m_io_cntxt);
    m_acceptor   = std::make_unique<boost::asio::ip::tcp::acceptor>(*m_io_cntxt);

    m_received_buffer.resize(4096);

    // Creating threads for receiving and sending data
    for(short i = 0; i < THREAD_NR; ++i)
        m_threads.create_thread(boost::bind(&Server::workerThread, this));
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

    try
    {
        // Finding the LAN IP address on Linux/Windows (only wi-fi or ethernet)
        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

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
                QList<QNetworkAddressEntry> entries = iface.addressEntries();
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

        if(m_endpoint)
        {
            boost::system::error_code ec;

            // Making the tcp connection
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

            m_acceptor->async_accept(*m_sckt,
                                     boost::bind(&Server::onAccept,
                                                 this,
                                                 boost::asio::placeholders::error
                                                 )
                                     );

            emit this->listening_on(m_endpoint);
        }
        else
        {
            throw std::runtime_error("No valid m_endpoint found after scanning interfaces!");
        }
    }
    catch (const std::exception& e)
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

        // If at least one client is connected to the server, other clients will not be
        // able to connect until the server terminates the current connection and initiates a new one.
        m_acceptor->close();
    }
}


void Server::send(const std::vector<boost::uint8_t>& send_buffer) noexcept
{
    try
    {
        boost::asio::async_write(*m_sckt, boost::asio::buffer(send_buffer),
                                 [this](const boost::system::error_code& ec, const std::size_t bytes){
                                     if(ec)
                                     {
                                         emit this->connectionStatus("An error occurred while transmitting data.");
                                     }
                                 });
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
            m_sckt->async_read_some(boost::asio::buffer(m_received_buffer, m_received_buffer.size()),
                                  boost::bind(&Server::onRecv,
                                              this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred
                                              )
                                  );
    }
    catch (const std::exception& e)
    {
        emit this->connectionStatus(e.what());
    }
}

void Server::onRecv(const boost::system::error_code& ec, const size_t bytes) noexcept
{
    if(ec)
    {
        emit this->connectionStatus("Async_read_some error!");
        return;
    }

    std::string received_message(m_received_buffer.begin(), m_received_buffer.begin() + bytes);
    received_message = "<span style='color: green;'>CLIENT: </span>" + received_message + "<br>";

    emit message_received(received_message);

    this->recv();
}


void Server::closeConnection() noexcept
{
    m_serverStatus = false;

    boost::system::error_code ec;

    try {
        if(m_sckt->is_open())
        {
            m_sckt->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            m_sckt->close(ec);

            if(ec) emit connectionStatus("Error on socket shutdown/close!");
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
