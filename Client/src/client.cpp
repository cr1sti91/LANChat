#include "client.h"


Client::Client(QObject* parent) : QObject(parent),
                                  m_endpoint(nullptr),
                                  m_clientStatus(std::nullopt)
{
    m_io_cntxt   = std::make_unique<boost::asio::io_context>();
    m_work       = std::make_unique<boost::asio::io_service::work>(*m_io_cntxt);
    m_sckt       = std::make_unique<boost::asio::ip::tcp::socket>(*m_io_cntxt);

    m_received_buffer.resize(4096);

    for(short i = 0; i < THREAD_NR; ++i)
        m_threads.create_thread(boost::bind(&Client::workerThread, this));
}

void Client::workerThread() noexcept
{
    while(true)

    {
        try
        {
            boost::system::error_code ec;
            m_io_cntxt->run(ec);

            if(ec)
            {
                emit this->connectionStatus("Error workerThread");
            }
            break;
        }
        catch(const std::exception& e)
        {
            emit this->connectionStatus("Exception workerThread");
        }
    }
}

Client::~Client()
{
    if(m_clientStatus.has_value() && m_clientStatus.value())
        this->finish();

    m_threads.join_all();
}

const std::optional<std::atomic<bool>> &Client::is_working() const noexcept
{
    return m_clientStatus;
}

void Client::connect(const char* ip_address, const unsigned port) noexcept
{
    try
    {
        m_endpoint = std::make_shared<boost::asio::ip::tcp::endpoint>(
                boost::asio::ip::make_address(ip_address), port
            );

        m_sckt->async_connect(*m_endpoint,
                              boost::bind(&Client::onConnect,
                                          this,
                                          boost::asio::placeholders::error
                                          )
                             );
    }
    catch (const std::exception& e)
    {
        emit this->connectionStatus("  Connection failed (exception)!");
    }
}

void Client::onConnect(const boost::system::error_code &ec) noexcept
{
    if(ec)
    {
        emit this->connectionStatus("  Connection failed (error)!");
    }
    else
    {
        m_clientStatus = true;
        emit this->connectionStatus("  Connected!");
    }
}


void Client::send(const std::vector<boost::uint8_t>& send_buffer) noexcept
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


void Client::recv() noexcept
{
    try
    {
        if(m_clientStatus.has_value() && m_clientStatus.value())
            m_sckt->async_read_some(boost::asio::buffer(m_received_buffer, m_received_buffer.size()),
                                  boost::bind(&Client::onRecv,
                                              this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred
                                              )
                                  );
    }
    catch(const std::exception& e)
    {
        if(m_clientStatus.has_value() && m_clientStatus.value())
            emit this->connectionStatus(e.what());
    }
}

void Client::onRecv(const boost::system::error_code& ec, const size_t bytes)   noexcept
{
    if(ec)
    {
        if(m_clientStatus.has_value() && m_clientStatus.value())
            emit this->connectionStatus("Async_read_some error!");

        return;
    }

    std::string received_message(m_received_buffer.begin(), m_received_buffer.begin() + bytes);

    emit this->message_received(received_message);

    if(m_clientStatus.has_value() && m_clientStatus.value())
        this->recv();
}


void Client::closeConnection() noexcept
{
    m_clientStatus = false;

    boost::system::error_code ec;

    try
    {
        if(m_sckt->is_open())
        {
            m_sckt->shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            m_sckt->close(ec);

            if(ec) emit this->connectionStatus("Error on socket shutdown/close!");
        }
    }
    catch (const std::exception& e)
    {
        emit this->connectionStatus(e.what());
    }
}


void Client::finish() noexcept
{
    this->closeConnection();

    m_work.reset();
    m_io_cntxt->stop();
    m_threads.join_all();
}
