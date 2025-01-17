#include "client_mainwindow.h"

//   Fields are called through the this pointer for explicitness.

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
void MainWindow::addServerInfo()
{
    this->addLayouts();
    this->addStatusLable("Waiting for IP address and port...");

    this->ipAddressInput = new QLineEdit();
    this->portInput = new QLineEdit();
    this->connectButton = new QPushButton("Connect");

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

void MainWindow::getServerInfo()
{
    if(this->centralWidget)
    {
        disconnect(this->connectButton, &QPushButton::clicked, nullptr, nullptr);

        delete this->centralWidget;
        this->centralWidget = nullptr;

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

        connect(this->connectButton, &QPushButton::clicked, this, [this](){
            this->serverIPaddress = this->ipAddressInput->text().toStdString();
            this->serverPort = this->portInput->text().toStdString();

            this->startConnection();
        });
    }

    connect(this->client, &Client::connectionStatus, this, &MainWindow::connectionStatus);
}

void MainWindow::setPalettes()
{
    this->windowPalette = new QPalette(QPalette::Window, Qt::white);
    this->buttonPalette = new QPalette();

    this->buttonPalette->setColor(QPalette::Button, Qt::white);
    this->buttonPalette->setColor(QPalette::ButtonText, Qt::black);

    this->setPalette(*windowPalette);
    this->setWindowTitle(MainWindow::WINDOWNAME);
}

void MainWindow::addLayouts()
{
    this->centralWidget = new QWidget();
    setCentralWidget(this->centralWidget);

    this->verticalLayout =  new QVBoxLayout(this->centralWidget);
    this->orizontalLayout = new QHBoxLayout();
}

void MainWindow::addMenu()
{
    this->quitAction = new QAction("Quit", this);
    this->connectAction = new QAction("Connect", this);

    this->fileMenu = menuBar()->addMenu("App");
    this->connectionMenu = menuBar()->addMenu("Connect");

    this->fileMenu->addAction(this->quitAction);
    this->connectionMenu->addAction(this->connectAction);

    connect(this->quitAction, &QAction::triggered, this, [this](){
        QApplication::quit();
    });

    connect(this->connectAction, &QAction::triggered, this, &MainWindow::getServerInfo);
}

void MainWindow::addStatusLable(const char* status)
{
    this->connectionStatusLabel = new QLabel(status);

    QPalette labelPalette;

    labelPalette.setColor(QPalette::WindowText, [status](){
        if(!std::strcmp(status, "  Connected!"))
            return Qt::green;
        if(!std::strcmp(status, "Waiting for IP address and port..."))
            return Qt::white;
        return Qt::red;
    }());

    this->connectionStatusLabel->setPalette(labelPalette);

    this->connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->connectionStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->verticalLayout->addWidget(this->connectionStatusLabel, 1, Qt::AlignTop);
    this->connectionStatusLabel->show();
}

void MainWindow::addUserInput()
{
    this->userInput = new QLineEdit();
    this->userInput->setPlaceholderText("Type...");

    this->orizontalLayout->addWidget(this->userInput);
    this->userInput->show();

    this->sendButton = new QPushButton();
    this->sendButton->setText("Send");

    // this->sendButton->setPalette(*this->buttonPalette);

    this->orizontalLayout->addWidget(this->sendButton);
    this->sendButton->show();

    this->verticalLayout->addLayout(this->orizontalLayout);

    connect(this->sendButton, &QPushButton::clicked, this, &MainWindow::sendingMessages);
}

void MainWindow::addMessagesLabel()
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

void MainWindow::startConnection()
{
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

void MainWindow::onConnection(const char* status)
{
    delete this->centralWidget; // Upon this destruction all child widgets are destroyed
    this->centralWidget = nullptr;

    this->addLayouts();
    this->addStatusLable(status);
    this->addMessagesLabel();
    this->addUserInput();

    this->client->recv(this->messagesLabel);
}

void MainWindow::sendingMessages()
{
    std::string input = this->userInput->text().toStdString();

    if(input.empty())
        return;

    input = "CLIENT: " + input + '\n';

    // Adding text to the messageLabel
    this->messageLabelMutex.lock();
    messagesLabel->setText(messagesLabel->text() + QString::fromStdString(input));
    this->messageLabelMutex.unlock();

    std::copy(input.begin(), input.end(), std::back_inserter(this->send_buffer));

    this->userInput->clear();

    this->client->send(this->send_buffer);
    this->send_buffer.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE SLOTS
///
void MainWindow::connectionStatus(const char* status)
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
                              (!std::strcmp(status, "Waiting for IP address and port...")? Qt::white : Qt::red));
        this->connectionStatusLabel->setPalette(labelPalette);
    }

    qDebug() << status;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS
///
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          client(new Client(this)),
                                          clientThread(nullptr)
{
    // this->setPalettes();
    this->addMenu();
}

MainWindow::~MainWindow()
{
    this->client->finish();

    if(this->centralWidget)
        delete centralWidget;
}

QSize MainWindow::sizeHint() const
{
    return QSize(800, 500);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
