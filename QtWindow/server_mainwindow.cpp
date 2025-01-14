#include "server_mainwindow.h"
#include <QDebug>

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
    this->setWindowTitle(MainWindow::windowName);
}

void MainWindow::addLayouts()
{
    this->centralWidget = new QWidget(this);
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

    connect(this->listenAction, &QAction::triggered, this, [this](){

        this->serverThread = new boost::thread(&Server::listen, this->server);

        this->addStatusLable();

        connect(this->server.get(), &Server::connectionStatus, this, &MainWindow::connectionStatus);

        emit serverListening();
    });

    this->fileMenu = menuBar()->addMenu("File");
    this->listenMenu = menuBar()->addMenu("Listen");

    this->fileMenu->addAction(this->quitAction);
    this->listenMenu->addAction(this->listenAction);
}

void MainWindow::addStatusLable()
{
    this->connectionStatusLabel = new QLabel(this);
    this->connectionStatusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->connectionStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->verticalLayout->addWidget(this->connectionStatusLabel, 1, Qt::AlignTop);
    this->connectionStatusLabel->show();

    connect(this->server.get(), &Server::listening_on, this, &MainWindow::setStatusLabel);
}

void MainWindow::addUserInput()
{
    this->userInput = new QLineEdit(this);
    this->userInput->setPlaceholderText("Type...");

    this->orizontalLayout->addWidget(this->userInput);
    this->userInput->show();

    this->sendButton = new QPushButton(this);
    this->sendButton->setText("Send");

    // this->sendButton->setPalette(*this->buttonPalette);

    this->orizontalLayout->addWidget(this->sendButton);
    this->sendButton->show();

    this->verticalLayout->addLayout(this->orizontalLayout);

    connect(this->sendButton, &QPushButton::clicked, this, [this](){

        std::string input = this->userInput->text().toStdString();

        if(input.empty())
            return;

        input = "SERVER: " + input + '\n';

        std::copy(input.begin(), input.end(), std::back_inserter(this->send_buffer));

        this->userInput->clear();

        this->server->send(this->send_buffer);
        this->send_buffer.clear();
    });
}

void MainWindow::addMessagesLabel()
{
    this->messagesLabel = new QLabel(this);
    this->messagesLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    this->messagesLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    this->verticalLayout->addWidget(this->messagesLabel, Qt::AlignCenter);
    this->messagesLabel->show();
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PRIVATE SLOTS
///
void MainWindow::setStatusLabel(boost::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint)
{
    QString ipAndPort = "  Listening on IP address " + QString::fromStdString(endpoint->address().to_string())
                        + " and port "+ QString::number(endpoint->port()) + "...";

    this->connectionStatusLabel->setText(ipAndPort);
}

void MainWindow::connectionStatus(const char* status)
{
    if(this->serverThread != nullptr)
    {
        // serverThread is deleted because the attempt to connect has finished.
        delete this->serverThread;
        this->serverThread = nullptr;

        this->connectionStatusLabel->setText(status);
        QPalette labelPalette;
        labelPalette.setColor(QPalette::WindowText, (!std::strcmp(status, "  Connected!"))? Qt::green : Qt::red);
        this->connectionStatusLabel->setPalette(labelPalette);

        this->addMessagesLabel();
        this->addUserInput();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////
/// PUBLIC METHODS
///
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          server(boost::make_shared<Server>()),
                                          serverThread(nullptr)
{
    // this->setPalettes();
    this->addLayouts();
    this->addMenu();
}

MainWindow::~MainWindow()
{
    this->server->finish();
}

QSize MainWindow::sizeHint() const
{
    return QSize(800, 500);
}
//////////////////////////////////////////////////////////////////////////////////////////////////
