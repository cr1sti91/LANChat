#include "client_mainwindow.h"

//   Fields are called through the this pointer for explicitness.

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
void CMainWindow::resetAtributes()
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
    ipAddressInput        = nullptr;
    portInput             = nullptr;
    connectButton         = nullptr;
}

void CMainWindow::addServerInfo()
{
    this->addLayouts();
    this->addStatusLable("Waiting for IP address and port...");

    this->ipAddressInput = new QLineEdit();
    this->portInput = new QLineEdit();
    this->connectButton = new QPushButton("Connect");
    this->connectButton->setPalette(*this->widgetsPalette);

    this->ipAddressInput->setFixedHeight(30);
    this->portInput->setFixedHeight(30);
    this->connectButton->setFixedHeight(40);

    this->ipAddressInput->setPlaceholderText("Type the server IP address...");
    this->portInput->setPlaceholderText("Type the server port...");

    this->verticalLayout->addWidget(this->ipAddressInput);
    this->verticalLayout->addWidget(this->portInput);
    this->verticalLayout->addWidget(this->connectButton);

    this->verticalLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}


void CMainWindow::addPalettes()
{
    this->widgetsPalette = new QPalette;

    // For the buttons
    this->widgetsPalette->setColor(QPalette::Button, Qt::blue);
    this->widgetsPalette->setColor(QPalette::ButtonText, Qt::black);

    // For the scroll area
    this->widgetsPalette->setColor(QPalette::Base, Qt::blue);

    this->setWindowTitle(CMainWindow::WINDOWNAME);
}

void CMainWindow::addLayouts()
{
    this->centralWidget = new QWidget();
    setCentralWidget(this->centralWidget);

    this->verticalLayout =  new QVBoxLayout(this->centralWidget);
    this->orizontalLayout = new QHBoxLayout();
}

void CMainWindow::addMenu()
{
    this->appMenu        = menuBar()->addMenu("App");
    this->connectionMenu = menuBar()->addMenu("Connect");
    this->optionsMenu    = menuBar()->addMenu("Options");

    this->quitAction          = new QAction("Quit", this);
    this->connectAction       = new QAction("Connect", this);
    this->clearMessagesAction = new QAction("Clear", this);

    this->appMenu->addAction(this->quitAction);
    this->connectionMenu->addAction(this->connectAction);
    this->optionsMenu->addAction(this->clearMessagesAction);

    connect(this->quitAction, &QAction::triggered, this, [this](){ QApplication::quit(); });
    connect(this->connectAction, &QAction::triggered, this, &CMainWindow::getServerInfo);
    connect(this->clearMessagesAction, &QAction::triggered, this, &CMainWindow::clearMessages);
}

void CMainWindow::addStatusLable(const char* status)
{
    this->connectionStatusLabel = new QLabel(status);

    QPalette labelPalette;

    labelPalette.setColor(QPalette::WindowText, [status](){
        if(!std::strcmp(status, "  Connected!"))
            return Qt::green;
        if(!std::strcmp(status, "Waiting for IP address and port..."))
            return Qt::cyan;
        return Qt::red;
    }());

    this->connectionStatusLabel->setPalette(labelPalette);

    this->connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->connectionStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->verticalLayout->addWidget(this->connectionStatusLabel, 1, Qt::AlignTop);
    this->connectionStatusLabel->show();
}

void CMainWindow::addUserInput()
{
    this->userInputLine = new QLineEdit();
    this->userInputLine->setPlaceholderText("Type...");

    this->orizontalLayout->addWidget(this->userInputLine);
    this->userInputLine->show();

    this->sendButton = new QPushButton();
    this->sendButton->setText("Send");

    this->sendButton->setPalette(*this->widgetsPalette);

    this->orizontalLayout->addWidget(this->sendButton);
    this->sendButton->show();

    this->verticalLayout->addLayout(this->orizontalLayout);

    // Messages are sent either when the 'sendButton' button or the Enter key is pressed.
    connect(this->sendButton, &QPushButton::clicked, this, &CMainWindow::sendingMessages);
    connect(this->userInputLine, &QLineEdit::returnPressed, this, &CMainWindow::sendingMessages);
}

void CMainWindow::addMessagesLabel()
{
    this->messagesLabel = new QLabel();
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

void CMainWindow::startConnection()
{
    // If the client is connected, to initiate a new connection, it is
    // necessary to close the previous one.
    if(this->client->is_working().has_value())
    {
        if(this->client->is_working())
        {
            if(this->clientThread != nullptr)
            {
                this->clientThread.reset(nullptr);
            }

            this->client->closeConnection();
        }
    }

    this->clientThread =
        std::make_unique<boost::thread>(boost::bind(&Client::connect,
                                                    this->client,
                                                    this->serverIPaddress.c_str(),
                                                    std::atoi(this->serverPort.c_str())
                                                    )
                                        );
}

void CMainWindow::onConnection(const char* status)
{
    delete this->centralWidget; // Upon this destruction all child widgets are destroyed
    this->resetAtributes();

    this->addLayouts();
    this->addStatusLable(status);
    this->addMessagesLabel();
    this->addUserInput();

    this->client->recv(boost::bind(&CMainWindow::displayMessage, this, boost::placeholders::_1));
}

void CMainWindow::displayMessage(const std::string &message)
{
    this->messageLabelMutex.lock();
    messagesLabel->setText(messagesLabel->text() + QString::fromStdString(message));

    // Set scroll bar in the bottom position
    this->messagesLabelScroll->verticalScrollBar()->setValue(
        messagesLabelScroll->verticalScrollBar()->maximum()
        );
    this->messageLabelMutex.unlock();
}



//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE SLOTS
///

void CMainWindow::getServerInfo()
{
    if(this->centralWidget)
    {
        delete this->centralWidget;
        this->resetAtributes();

        this->addServerInfo();

        if(this->client->is_working().has_value())
        {
            if(this->client->is_working())
            {
                if(this->clientThread != nullptr)
                {
                    this->clientThread.reset(nullptr);
                }

                this->client->closeConnection();
            }
        }
    }
    else
    {
        this->addServerInfo();
        connect(this->client, &Client::connectionStatus, this, &CMainWindow::connectionStatus);
    }

    connect(this->connectButton, &QPushButton::clicked, this, [this](){
        this->serverIPaddress = this->ipAddressInput->text().toStdString();
        this->serverPort = this->portInput->text().toStdString();

        this->startConnection();
    });
}


void CMainWindow::connectionStatus(const char* status)
{
    if(this->clientThread != nullptr)
    {
        // serverThread is deleted because the attempt to connect has finished.
        this->clientThread->join();
        this->clientThread.reset(nullptr);
        this->clientThread = nullptr;
    }

    if(!std::strcmp(status, "  Connected!"))
    {
        this->onConnection(status);
    }
    else
    {
        this->connectionStatusLabel->setText(status);

        QPalette labelPalette;
        labelPalette.setColor(QPalette::WindowText,
                              (!std::strcmp(status, "Waiting for IP address and port...")? Qt::cyan : Qt::red));
        this->connectionStatusLabel->setPalette(labelPalette);
    }
}

void CMainWindow::sendingMessages()
{
    std::string input = this->userInputLine->text().toStdString();

    if(input.empty())
        return;

    std::copy(input.begin(), input.end(), std::back_inserter(this->send_buffer));

    this->userInputLine->clear();

    this->client->send(this->send_buffer);
    this->send_buffer.clear();

    input = "CLIENT: " + input + '\n';

    // Adding text to the messageLabel
    this->displayMessage(input);
}

void CMainWindow::clearMessages()
{
    if(this->messagesLabel)
        this->messagesLabel->clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS
///
CMainWindow::CMainWindow(QWidget *parent) : QMainWindow(parent),
                                            client(new Client(this)),
                                            clientThread(nullptr)
{
    this->addPalettes();
    this->addMenu();
}

CMainWindow::~CMainWindow()
{
    this->client->finish();

    if(this->centralWidget)
    {
        delete centralWidget;
        this->resetAtributes();
    }
}

QSize CMainWindow::sizeHint() const
{
    return QSize(800, 500);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
