#include "server_mainwindow.h"
#include <QDebug>

void MainWindow::addMenu()
{
    quitAction = new QAction("Quit", this);
    listenAction = new QAction("Listen", this);

    connect(quitAction, &QAction::triggered, this, [this](){
        QApplication::quit();
    });

    connect(listenAction, &QAction::triggered, this, [this](){

        serverThread = new boost::thread(&Server::listen, server);
        connect(server.get(), &Server::connectionStatus, this, &MainWindow::connectionStatus);

        emit serverListening();
    });

    fileMenu = menuBar()->addMenu("File");
    listenMenu = menuBar()->addMenu("Listen");

    fileMenu->addAction(quitAction);
    listenMenu->addAction(listenAction);
}

void MainWindow::addStatusLable()
{
    connectionStatusLabel = new QLabel(this);
    connectionStatusLabel->move(0, fileMenu->height());

    connect(server.get(), &Server::listening_on, this, &MainWindow::setStatusLabel);
}

void MainWindow::addUserInput()
{
    userInput = new QLineEdit("Type...", this);
    userInput->setFixedHeight(40);

    userInput->move(0, 200);
}

void MainWindow::setStatusLabel(boost::shared_ptr<boost::asio::ip::tcp::endpoint> endpoint)
{
    QString ipAndPort = "Listening on IP address " + QString::fromStdString(endpoint->address().to_string())
                        + " and port "+ QString::number(endpoint->port()) + "...";

    QFont font("Times", 14);
    QFontMetrics metrics(font);

    connectionStatusLabel->setText(ipAndPort);
    connectionStatusLabel->setFixedWidth(metrics.horizontalAdvance(ipAndPort) + 10);
}

void MainWindow::connectionStatus(const QString& status)
{
    if(serverThread != nullptr)
    {
        delete serverThread;
        serverThread = nullptr;

        connectionStatusLabel->setText(status);
        QPalette labelPalette;
        labelPalette.setColor(QPalette::WindowText, (status == "  Connected!")? Qt::green : Qt::red);
        connectionStatusLabel->setPalette(labelPalette);

        this->addUserInput();
    }
}


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), server(boost::make_shared<Server>()),
                                          serverThread(nullptr)
{
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    this->addMenu();
    this->addStatusLable();
}

MainWindow::~MainWindow()
{
    server->finish();
}

QSize MainWindow::sizeHint() const
{
    return QSize(700, 500);
}
