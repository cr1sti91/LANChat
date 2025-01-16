#include "server.h"

Server::Server(QObject* parent) : QObject(parent),
                                  received_buffer_index(0),
                                  endpoint(nullptr),
                                  serverStatus(std::nullopt)
{
    io_cntxt   = std::make_unique<boost::asio::io_context>();
    work       = std::make_unique<boost::asio::io_service::work>(*io_cntxt);
    sckt       = std::make_unique<boost::asio::ip::tcp::socket>(*io_cntxt);
    acceptor   = std::make_unique<boost::asio::ip::tcp::acceptor>(*io_cntxt);

    received_buffer.resize(4096);

    for(short i = 0; i < THREAD_NR; ++i)
        threads.create_thread(boost::bind(&Server::workerThread, this));
}

void Server::workerThread() noexcept
{
    while(true)

    {
        try
        {
            boost::system::error_code ec;
            io_cntxt->run(ec);

            if(ec)
            {
                emit connectionStatus("Error workerThread");
            }
            break;
        }
        catch(const std::exception& e)
        {
            emit connectionStatus("Exception workerThread");
        }
    }
}

Server::~Server()
{
}

const std::optional<std::atomic<bool>> &Server::is_working() const
{
    return this->serverStatus;
}

void Server::listen() noexcept
{
    this->serverStatus = true;

    try
    {
        // Finding the LAN IP address on Linux/Windows (only wi-fi or ethernet)
        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

        for (const QNetworkInterface &iface : interfaces)
        {
            const QString ifaceName = iface.name();
            boost::asio::ip::address private_IP_address;
            bool wifi_or_ethernet = false;


#ifdef __linux__
            wifi_or_ethernet = ifaceName.startsWith("wl") ||
                               ifaceName.startsWith("en") ||
                               ifaceName.startsWith("eth");
#elif _WIN32
            wifi_or_ethernet = ifaceName.contains("Wi-Fi", Qt::CaseInsensitive)) ||
                               ifaceName.startsWith("Ethernet", Qt::CaseInsensitive);
#endif
            if (wifi_or_ethernet)
            {
                QList<QNetworkAddressEntry> entries = iface.addressEntries();
                for (const QNetworkAddressEntry &entry : entries)
                {
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol && !entry.ip().isLoopback())
                    {
                        private_IP_address = boost::asio::ip::make_address(entry.ip().toString().toStdString());
                        endpoint = std::make_shared<boost::asio::ip::tcp::endpoint>(private_IP_address,
                                                                                      SERVER_PORT);
                        break;
                    }
                }

                if(endpoint)
                    break;
            }
        }

        if(endpoint)
        {
            boost::system::error_code ec;

            // Making the tcp connection
            acceptor->open(endpoint->protocol());
            acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor->bind(*endpoint, ec);

            if(ec)
            {
                emit connectionStatus("Invalid endpoint (probably the port is occupied by another instance)!");
                endpoint.reset();
                endpoint = nullptr;
                return;
            }

            acceptor->listen(boost::asio::socket_base::max_connections);
            acceptor->async_accept(*sckt, boost::bind(&Server::onAccept, this, _1));

            emit listening_on(endpoint);
        }
        else
        {
            throw std::runtime_error("No valid endpoint found after scanning interfaces!");
        }
    }
    catch (const std::exception& e)
    {
        emit connectionStatus(e.what());
    }
}

void Server::onAccept(const boost::system::error_code &ec) noexcept
{
    if(ec)
    {
        emit connectionStatus("  Connection failed!");
    }
    else
    {
        emit connectionStatus("  Connected!");
    }
}


void Server::send(const std::vector<boost::uint8_t>& send_buffer) noexcept
{
    boost::asio::async_write(*sckt, boost::asio::buffer(send_buffer),
                             [this](const boost::system::error_code& ec, const std::size_t bytes){
                               if(ec)
                                 {
                                     emit connectionStatus("An error occurred while transmitting data.");
                                 }
                             });
}


void Server::recv(QLabel* messageLabel) noexcept
{
    sckt->async_read_some(boost::asio::buffer(received_buffer, received_buffer.size()),
                          boost::bind(&Server::onRecv, this, _1, _2, messageLabel));
}

void Server::onRecv(const boost::system::error_code& ec, const size_t bytes, QLabel *messageLabel) noexcept
{
    if(ec)
    {
        emit connectionStatus("Async_read_some error!");
        return;
    }

    std::string received_message(received_buffer.begin(), received_buffer.begin() + bytes);

    this->mutex.lock();
    messageLabel->setText(messageLabel->text() + "CLIENT: " + QString::fromStdString(received_message));
    this->mutex.unlock();

    recv(messageLabel);
}


void Server::closeConnection() noexcept
{
    boost::system::error_code ec;

    try {
        if(sckt->is_open())
        {
            sckt->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            sckt->close(ec);

            if(ec) emit connectionStatus("Error on socket shutdown/close!");
        }

        if(acceptor->is_open())
            acceptor->close();

    }
    catch (const std::exception& e)
    {
        emit connectionStatus(e.what());
    }

    this->serverStatus = false;
}


void Server::finish() noexcept
{
    this->closeConnection();

    work.reset();
    io_cntxt->stop();
    threads.join_all();

    this->serverStatus = false;
}
