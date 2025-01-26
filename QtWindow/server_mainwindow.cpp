#include "server_mainwindow.h"

//   Fields are called through the this pointer for explicitness.

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
void SMainWindow::resetAtributes()
{
    connectionStatusLabel = nullptr;
    messagesLabel         = nullptr;

    messagesLabelScroll   = nullptr;
    messagesLabelLayout   = nullptr;

    userInputLine         = nullptr;

    centralWidget         = nullptr;
    verticalLayout        = nullptr;
    orizontalLayout       = nullptr;

    sendButton            = nullptr;
}

void SMainWindow::setPalettes()
{
    this->widgetsPalette = new QPalette;

    // For the buttons
    this->widgetsPalette->setColor(QPalette::Button, Qt::blue);
    this->widgetsPalette->setColor(QPalette::ButtonText, Qt::black);

    // For the scroll area
    this->widgetsPalette->setColor(QPalette::Base, Qt::blue);

    this->setWindowTitle(SMainWindow::WINDOWNAME);
}

void SMainWindow::addLayouts()
{
    this->centralWidget = new QWidget();
    setCentralWidget(this->centralWidget);

    this->verticalLayout =  new QVBoxLayout(this->centralWidget);
}

void SMainWindow::addMenu()
{
    this->appMenu     = menuBar()->addMenu("App");
    this->listenMenu  = menuBar()->addMenu("Connection");
    this->optionsMenu = menuBar()->addMenu("Options");

    this->quitAction          = new QAction("Quit", this);
    this->listenAction        = new QAction("Listen", this);
    this->clearMessagesAction = new QAction("Clear", this);

    this->appMenu->addAction(this->quitAction);
    this->listenMenu->addAction(this->listenAction);
    this->optionsMenu->addAction(this->clearMessagesAction);

    connect(this->quitAction, &QAction::triggered, this, [this](){ QApplication::quit(); });
    connect(this->listenAction, &QAction::triggered, this, &SMainWindow::startListening);
    connect(this->clearMessagesAction, &QAction::triggered, this, &SMainWindow::clearMessages);
}

void SMainWindow::addStatusLable()
{
    this->connectionStatusLabel = new QLabel(this->centralWidget);
    this->connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->connectionStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->verticalLayout->addWidget(this->connectionStatusLabel, 1, Qt::AlignTop);
    this->connectionStatusLabel->show();


    connect(this->server, &Server::listening_on, this, &SMainWindow::setStatusLabel);
}

void SMainWindow::addUserInput()
{
    this->orizontalLayout = new QHBoxLayout();
    this->verticalLayout->addLayout(this->orizontalLayout);

    this->userInputLine = new QLineEdit(this->centralWidget);
    this->userInputLine->setPlaceholderText("Type...");

    this->orizontalLayout->addWidget(this->userInputLine);
    this->userInputLine->show();

    this->sendButton = new QPushButton(this->centralWidget);
    this->sendButton->setText("Send");

    this->sendButton->setPalette(*this->widgetsPalette);

    this->orizontalLayout->addWidget(this->sendButton);
    this->sendButton->show();

    // Messages are sent either when the 'sendButton' button or the Enter key is pressed.
    connect(this->sendButton, &QPushButton::clicked, this, &SMainWindow::sendingMessages);
    connect(this->userInputLine, &QLineEdit::returnPressed, this, &SMainWindow::sendingMessages);
}

void SMainWindow::addMessagesLabel()
{
    this->messagesLabel = new QLabel(this->centralWidget);
    this->messagesLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->messagesLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->messagesLabel->setAlignment(Qt::AlignBottom);

    this->messagesLabelScroll = new QScrollArea(this->centralWidget);
    this->messagesLabelScroll->setWidget(this->messagesLabel);
    this->messagesLabelScroll->setWidgetResizable(true);
    this->messagesLabelScroll->setPalette(*this->widgetsPalette);

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
            this->resetAtributes();

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
    this->displayMessage(input);
}

void SMainWindow::displayMessage(const std::string &message)
{
    this->messageLabelMutex.lock();
    messagesLabel->setText(messagesLabel->text() + QString::fromStdString(message));

    // Set scroll bar in the bottom position
    this->messagesLabelScroll->verticalScrollBar()->setValue(
        messagesLabelScroll->verticalScrollBar()->maximum()
        );
    this->messageLabelMutex.unlock();
}

void SMainWindow::cleanup()
{
    delete this->widgetsPalette;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE SLOTS
///
void SMainWindow::setStatusLabel(const std::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint)
{
    QString ipAndPort = "  Listening on IP address " + QString::fromStdString(endpoint->address().to_string())
                        + " and port "+ QString::number(endpoint->port()) + "...";

    QPalette statusLabelPalette;
    statusLabelPalette.setColor(QPalette::WindowText, Qt::cyan);
    this->connectionStatusLabel->setPalette(statusLabelPalette);

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

        this->server->recv(boost::bind(&SMainWindow::displayMessage, this, boost::placeholders::_1));
    }
}

void SMainWindow::clearMessages()
{
    if(this->messagesLabel)
        this->messagesLabel->clear();
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
    {
        delete this->centralWidget;
        this->resetAtributes();
    }

    this->cleanup();
}

QSize SMainWindow::sizeHint() const
{
    return QSize(800, 500);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
