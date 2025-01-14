#include "server.h"

Server::Server() : received_buffer_index(0), end(false), endpoint(nullptr)
{
    io_cntxt   = boost::make_shared<boost::asio::io_context>();
    work       = boost::make_shared<boost::asio::io_service::work>(*io_cntxt);
    sckt       = boost::make_shared<boost::asio::ip::tcp::socket>(*io_cntxt);
    acceptor   = boost::make_shared<boost::asio::ip::tcp::acceptor>(*io_cntxt);

    received_buffer.resize(4096);

    for(short i = 0; i < THREAD_NR; ++i)
        threads.create_thread(boost::bind(&Server::workerThread, this, i));
}

void Server::listen() noexcept
{
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
                        endpoint = boost::make_shared<boost::asio::ip::tcp::endpoint>(private_IP_address,
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
            // Making the tcp connection
            acceptor->open(endpoint->protocol());
            acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            acceptor->bind(*endpoint);
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

        recv();
    }
}

void Server::workerThread(const std::size_t index)
{
    while(true)
    {
        try
        {
            boost::system::error_code ec;
            io_cntxt->run(ec);

            if(ec)
            {
                emit error_workerThread();
            }
            break;
        }
        catch(const std::exception& e)
        {
            emit exception_workerThread();
        }
    }
}

void Server::send(const std::vector<boost::uint8_t>& send_buffer) noexcept
{
    boost::asio::async_write(*sckt, boost::asio::buffer(send_buffer),
                             boost::bind(&Server::onSend, this, _1, _2));
}

void Server::onSend(const boost::system::error_code &ec, std::size_t n_bytes) noexcept
{
    if(ec)
    {
        emit onSend_error();
        return;
    }

    // send_buffer.clear();


    // //////////////////////////////////////////////////////////////////////////////
    // std::string to_send;

    // if(to_send == "exit")
    // {
    //     end = true;
    //     return;
    // }

    // to_send += '\n';
    // to_send = "Server: " + to_send;

    // std::copy(to_send.begin(), to_send.end(), std::back_inserter(send_buffer));

    // if(!send_buffer.empty())
    // {
    //     boost::asio::async_write(*sckt, boost::asio::buffer(send_buffer),
    //                              boost::bind(&Server::onSend, this, _1, _2));
    // }
}

void Server::recv() noexcept
{

}

void Server::onRecv() noexcept
{

}

void Server::finish() noexcept
{
    try {
        if(sckt->is_open())
        {
            sckt->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
            sckt->close();
        }

        if(acceptor->is_open())
            acceptor->close();

        work.reset();
        io_cntxt->stop();
        threads.join_all();
    }
    catch (const std::exception& e)
    {
        qDebug() << "exception in finish() : " << e.what();
    }
}
