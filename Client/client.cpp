#include "client.h"


Client::Client(QObject* parent) : QObject(parent),
                                  received_buffer_index(0),
                                  endpoint(nullptr),
                                  clientStatus(std::nullopt)
{
    this->io_cntxt   = std::make_unique<boost::asio::io_context>();
    this->work       = std::make_unique<boost::asio::io_service::work>(*io_cntxt);
    this->sckt       = std::make_unique<boost::asio::ip::tcp::socket>(*io_cntxt);

    this->received_buffer.resize(4096);

    for(short i = 0; i < THREAD_NR; ++i)
        this->threads.create_thread(boost::bind(&Client::workerThread, this));
}

void Client::workerThread() noexcept
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

Client::~Client()
{

    this->threads.join_all();
}

const std::optional<std::atomic<bool>> &Client::is_working() const
{
    return this->clientStatus;
}

void Client::connect(const char* ip_address, const unsigned port) noexcept
{
    this->clientStatus = true;

    try
    {
        this->endpoint = std::make_shared<boost::asio::ip::tcp::endpoint>(
                boost::asio::ip::make_address(ip_address), port
            );

        this->sckt->async_connect(*this->endpoint, boost::bind(&Client::onConnect, this, _1));
    }
    catch (const std::exception& e)
    {
        emit connectionStatus("  Connection failed (exception)!");
    }
}

void Client::onConnect(const boost::system::error_code &ec) noexcept
{
    if(ec)
    {
        emit connectionStatus("  Connection failed (error)!");
    }
    else
    {
        emit connectionStatus("  Connected!");
    }
}


void Client::send(const std::vector<boost::uint8_t>& send_buffer) noexcept
{

}


void Client::recv(QLabel* messageLabel) noexcept
{
    sckt->async_read_some(boost::asio::buffer(received_buffer, received_buffer.size()),
                          boost::bind(&Client::onRecv, this, _1, _2, messageLabel));
}

void Client::onRecv(const boost::system::error_code& ec, const size_t bytes, QLabel *messageLabel) noexcept
{
    if(ec)
    {
        emit connectionStatus("Async_read_some error!");
        return;
    }

    std::string received_message(received_buffer.begin(), received_buffer.begin() + bytes);

    this->mutex.lock();
    messageLabel->setText(messageLabel->text() + "SERVER: " + QString::fromStdString(received_message) + '\n');
    this->mutex.unlock();

    recv(messageLabel);
}


void Client::closeConnection() noexcept
{
    boost::system::error_code ec;

    try {
        if(sckt->is_open())
        {
            sckt->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            sckt->close(ec);

            if(ec) emit connectionStatus("Error on socket shutdown/close!");
        }
    }
    catch (const std::exception& e)
    {
        emit connectionStatus(e.what());
    }

    this->clientStatus = false;
}


void Client::finish() noexcept
{
    this->closeConnection();

    work.reset();
    io_cntxt->stop();
    threads.join_all();

    this->clientStatus = false;
}
