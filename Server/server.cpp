#include "server.h"


Server::Server() : received_buffer_index(0), end(false)
{
    io_cntxt   = boost::make_shared<boost::asio::io_context>();
    work       = boost::make_shared<boost::asio::io_service::work>(*io_cntxt);
    sckt       = boost::make_shared<boost::asio::ip::tcp::socket>(*io_cntxt);
    acceptor   = boost::make_shared<boost::asio::ip::tcp::acceptor>(*io_cntxt);

    received_buffer.resize(4096);

    for(short i = 0; i < thread_nr; ++i)
        threads.create_thread(boost::bind(&Server::workerThread, this, i));
}

void Server::listen(const char *address, const std::size_t port, const QLineEdit * const lineEdit) noexcept
{
    try
    {
        // DNS resolution if we have a remote host name.
        boost::asio::ip::tcp::resolver resolver(*io_cntxt);
        boost::asio::ip::tcp::resolver::query query(address, std::to_string(port));
        boost::asio::ip::tcp::endpoint endpoint = *(resolver.resolve(query));

        acceptor->open(endpoint.protocol());
        acceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor->bind(endpoint);
        acceptor->listen(boost::asio::socket_base::max_connections);
        acceptor->async_accept(*sckt, boost::bind(&Server::onAccept, this, _1, lineEdit));

        emit listening_on();
    }
    catch (const std::exception& e)
    {
        emit exception_listening_on();
    }

    finish();
}

void Server::onAccept(const boost::system::error_code &ec, const QLineEdit * const lineEdit) noexcept
{
    if(ec)
    {
        emit connection_failed();
    }
    else
    {
        emit connected();

        send(lineEdit);
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

void Server::send(const QLineEdit * const lineEdit) noexcept
{
    const char* message = "Connected!\n";
    size_t length = 11;
    std::vector<boost::uint8_t> output;
    std::copy((const boost::uint8_t*)message, (const boost::uint8_t*)message + length,
               std::back_inserter(send_buffer));

    boost::asio::async_write(*sckt, boost::asio::buffer(send_buffer),
                             boost::bind(&Server::onSend, this, _1, _2, lineEdit));
}

void Server::onSend(const boost::system::error_code &ec, std::size_t n_bytes, const QLineEdit* lineEdit) noexcept
{
    if(ec)
    {
        emit onSend_error();
        return;
    }

    send_buffer.clear();




    if(to_send == "exit")
    {
        end = true;
        return;
    }

    to_send += '\n';
    to_send = "Server: " + to_send;

    std::copy(to_send.begin(), to_send.end(), std::back_inserter(send_buffer));

    if(!send_buffer.empty())
    {
        boost::asio::async_write(*sckt, boost::asio::buffer(send_buffer),
                                 boost::bind(&Server::onSend, this, _1, _2));
    }
}

void Server::recv() noexcept
{

}

void Server::onRecv() noexcept
{

}

void Server::finish() noexcept
{
    work.reset();
    threads.join_all();

    sckt->shutdown(boost::asio::ip::tcp::socket::shutdown_both);
    sckt->close();
    acceptor->close();

    io_cntxt->stop();
}
