#include "server_mainwindow.h"

//   Fields are called through the this pointer for explicitness.

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
void SMainWindow::setPalettes()
{
    this->windowPalette = new QPalette(QPalette::Window, Qt::gray);
    this->buttonPalette = new QPalette();

    this->buttonPalette->setColor(QPalette::Button, Qt::blue);
    this->buttonPalette->setColor(QPalette::ButtonText, Qt::black);

    this->setPalette(*windowPalette);
    this->setWindowTitle(SMainWindow::WINDOWNAME);
}

void SMainWindow::addLayouts()
{
    this->centralWidget = new QWidget();
    setCentralWidget(this->centralWidget);

    this->verticalLayout =  new QVBoxLayout(this->centralWidget);
    this->orizontalLayout = new QHBoxLayout();
}

void SMainWindow::addMenu()
{
    this->quitAction = new QAction("Quit", this);
    this->listenAction = new QAction("Listen", this);

    connect(this->quitAction, &QAction::triggered, this, [this](){
        QApplication::quit();
    });

    connect(this->listenAction, &QAction::triggered, this, &SMainWindow::startListening);

    this->appMenu = menuBar()->addMenu("App");
    this->listenMenu = menuBar()->addMenu("Listen");

    this->appMenu->addAction(this->quitAction);
    this->listenMenu->addAction(this->listenAction);
}

void SMainWindow::addStatusLable()
{
    this->connectionStatusLabel = new QLabel();
    this->connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->connectionStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->verticalLayout->addWidget(this->connectionStatusLabel, 1, Qt::AlignTop);
    this->connectionStatusLabel->show();

    connect(this->server, &Server::listening_on, this, &SMainWindow::setStatusLabel);
}

void SMainWindow::addUserInput()
{
    this->userInputLine = new QLineEdit();
    this->userInputLine->setPlaceholderText("Type...");

    this->orizontalLayout->addWidget(this->userInputLine);
    this->userInputLine->show();

    this->sendButton = new QPushButton();
    this->sendButton->setText("Send");

    this->sendButton->setPalette(*this->buttonPalette);

    this->orizontalLayout->addWidget(this->sendButton);
    this->sendButton->show();

    this->verticalLayout->addLayout(this->orizontalLayout);

    connect(this->sendButton, &QPushButton::clicked, this, &SMainWindow::sendingMessages);
}

void SMainWindow::addMessagesLabel()
{
    this->messagesLabel = new QLabel();
    this->messagesLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->messagesLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->messagesLabel->setAlignment(Qt::AlignBottom);

    this->messagesLabelScroll = new QScrollArea(this->centralWidget);
    this->messagesLabelScroll->setWidget(this->messagesLabel);
    this->messagesLabelScroll->setWidgetResizable(true);

    this->messagesLabelLayout = new QHBoxLayout();
    this->messagesLabelLayout->addWidget(this->messagesLabelScroll);

    this->verticalLayout->addLayout(this->messagesLabelLayout, Qt::AlignCenter);
}

void SMainWindow::startListening()
{
    // If the server is connected, to initiate a new connection, it is
    // necessary to close the previous one.
    if(this->server->is_working().has_value())
    {
        if(this->server->is_working())
        {
            delete this->centralWidget;
            this->centralWidget = nullptr;

            if(this->serverThread != nullptr)
            {
                this->serverThread.reset(nullptr);
            }

            this->server->closeConnection();
        }
    }
    else
    {
        connect(this->server, &Server::connectionStatus, this, &SMainWindow::connectionStatus);
    }

    this->serverThread = std::make_unique<boost::thread>(&Server::listen, this->server);
    this->addLayouts();
    this->addStatusLable();
}

void SMainWindow::sendingMessages()
{
    std::string input = this->userInputLine->text().toStdString();

    if(input.empty())
        return;

    std::copy(input.begin(), input.end(), std::back_inserter(this->send_buffer));
    this->userInputLine->clear();

    this->server->send(this->send_buffer);
    this->send_buffer.clear();

    input = "SERVER: " + input + '\n';

    // Adding text to the messageLabel
    this->messageLabelMutex.lock();
    messagesLabel->setText(messagesLabel->text() + QString::fromStdString(input));
    this->messageLabelMutex.unlock();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE SLOTS
///
void SMainWindow::setStatusLabel(const std::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint)
{
    QString ipAndPort = "  Listening on IP address " + QString::fromStdString(endpoint->address().to_string())
                        + " and port "+ QString::number(endpoint->port()) + "...";


    this->connectionStatusLabel->setText(ipAndPort);

    // Disconnection from signal
    disconnect(this->server, &Server::listening_on, this, &SMainWindow::setStatusLabel);
}

void SMainWindow::connectionStatus(const char* status)
{
    if(this->serverThread != nullptr)
    {
        // serverThread is deleted because the attempt to connect has finished.
        this->serverThread->join();
        this->serverThread.reset(nullptr);
        this->serverThread = nullptr;
    }

    QPalette labelPalette;
    this->connectionStatusLabel->setText(status);

    labelPalette.setColor(QPalette::WindowText, (!std::strcmp(status, "  Connected!"))? Qt::green : Qt::red);
    this->connectionStatusLabel->setPalette(labelPalette);

    if(!std::strcmp(status, "  Connected!"))
    {
        this->addMessagesLabel();
        this->addUserInput();

        this->server->recv(this->messagesLabel);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS
///
SMainWindow::SMainWindow(QWidget *parent) : QMainWindow(parent),
                                            server(new Server(this)),
                                            serverThread(nullptr)
{
    this->setPalettes();
    this->addMenu();
}

SMainWindow::~SMainWindow()
{
    this->server->finish();

    if(this->centralWidget)
        delete centralWidget;
}

QSize SMainWindow::sizeHint() const
{
    return QSize(800, 500);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
