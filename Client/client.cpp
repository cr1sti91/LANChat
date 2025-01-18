#include "client.h"


Client::Client(QObject* parent) : QObject(parent),
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
        this->clientStatus = true;
        emit connectionStatus("  Connected!");
    }
}


void Client::send(const std::vector<boost::uint8_t>& send_buffer) noexcept
{
    boost::asio::async_write(*sckt, boost::asio::buffer(send_buffer),
                             [this](const boost::system::error_code& ec, const std::size_t bytes){
                                 if(ec)
                                 {
                                     emit connectionStatus("An error occurred while transmitting data.");
                                 }
                             });
}


void Client::recv(QLabel* messageLabel) noexcept
{
    try
    {
        boost::lock_guard<boost::mutex> lckgrd(this->mutex);

        if(this->clientStatus.has_value() && this->clientStatus.value())
            sckt->async_read_some(boost::asio::buffer(received_buffer, received_buffer.size()),
                                  boost::bind(&Client::onRecv, this, _1, _2, messageLabel));
    }
    catch(const std::exception& e)
    {
        emit connectionStatus(e.what());
    }
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

    if(this->clientStatus.has_value() && this->clientStatus.value())
        recv(messageLabel);
}


void Client::closeConnection() noexcept
{
    boost::lock_guard<boost::mutex> lckgrd(this->mutex);

    this->clientStatus = false;

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
}


void Client::finish() noexcept
{
    this->clientStatus = false;

    this->closeConnection();

    work.reset();
    io_cntxt->stop();
    threads.join_all();
}
