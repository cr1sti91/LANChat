#include "server_mainwindow.h"

//   Fields are called through the this pointer for explicitness.

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE METHODS
///
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
    this->listenAction = new QAction("Listen", this);

    connect(this->quitAction, &QAction::triggered, this, [this](){
        QApplication::quit();
    });

    connect(this->listenAction, &QAction::triggered, this, &MainWindow::startListening);

    this->fileMenu = menuBar()->addMenu("File");
    this->listenMenu = menuBar()->addMenu("Listen");

    this->fileMenu->addAction(this->quitAction);
    this->listenMenu->addAction(this->listenAction);
}

void MainWindow::addStatusLable()
{
    this->connectionStatusLabel = new QLabel();
    this->connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->connectionStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->verticalLayout->addWidget(this->connectionStatusLabel, 1, Qt::AlignTop);
    this->connectionStatusLabel->show();

    connect(this->server, &Server::listening_on, this, &MainWindow::setStatusLabel);
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

void MainWindow::startListening()
{
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
        connect(this->server, &Server::connectionStatus, this, &MainWindow::connectionStatus);
    }

    this->serverThread = std::make_unique<boost::thread>(&Server::listen, this->server);
    this->addLayouts();
    this->addStatusLable();
}

void MainWindow::sendingMessages()
{
    std::string input = this->userInput->text().toStdString();

    if(input.empty())
        return;

    input = "SERVER: " + input + '\n';

    // Adding text to the messageLabel
    this->messageLabelMutex.lock();
    messagesLabel->setText(messagesLabel->text() + QString::fromStdString(input));
    this->messageLabelMutex.unlock();

    std::copy(input.begin(), input.end(), std::back_inserter(this->send_buffer));

    this->userInput->clear();

    this->server->send(this->send_buffer);
    this->send_buffer.clear();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE SLOTS
///
void MainWindow::setStatusLabel(const std::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint)
{
    QString ipAndPort = "  Listening on IP address " + QString::fromStdString(endpoint->address().to_string())
                        + " and port "+ QString::number(endpoint->port()) + "...";


    this->connectionStatusLabel->setText(ipAndPort);

    // Disconnection from signal
    disconnect(this->server, &Server::listening_on, this, &MainWindow::setStatusLabel);
}

void MainWindow::connectionStatus(const char* status)
{
    if(this->serverThread != nullptr)
    {
        // serverThread is deleted because the attempt to connect has finished.
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
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          server(new Server(this)),
                                          serverThread(nullptr)
{
    // this->setPalettes();
    this->addMenu();
}

MainWindow::~MainWindow()
{
    this->server->finish();

    if(this->centralWidget)
        delete centralWidget;
}

QSize MainWindow::sizeHint() const
{
    return QSize(800, 500);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
